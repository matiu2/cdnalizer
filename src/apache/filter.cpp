/**
 * © Copyright 2014 Matthew Sherborne. All Rights Reserved.
 * License: Apache License, Version 2.0 (See LICENSE.txt)
 **/
#include "filter.hpp"

#include "../Rewriter.hpp"
#include "../Config.hpp"
#include "../Rewriter_impl.hpp"
#include "iterator.hpp"
#include "mod_cdnalizer.hpp"

#include <sstream>

extern "C" {

#include <apr_buckets.h>
#include <http_core.h>
#include <http_log.h>

APLOG_USE_MODULE(cdnalizer_module);

}

namespace cdnalizer {
namespace apache {

apr_status_t filter(ap_filter_t *filter, apr_bucket_brigade *bb) {
    // Just pass on empty brigades
    if (APR_BRIGADE_EMPTY(bb)) { return APR_SUCCESS; }

    // Get our current path from Apache
    std::string location{filter->r->uri};
    auto pos = location.rfind('/');
    if (pos != std::string::npos)
        location.resize(pos+1);

    Config* config = static_cast<Config*>(ap_get_module_config(filter->r->per_dir_config, &cdnalizer_module));

    // Work to be sent to the next filter on flush or ending
    BrigadeGuard completed_work{filter->r->pool, filter->c->bucket_alloc};

    // Called when we need to flush our completed work
    auto flush = [&]() {
        apr_status_t result = ap_pass_brigade(filter->next, completed_work);
        apr_brigade_cleanup(completed_work);
        return result;
    };

    // See if there's any work left over from last time
    apr_bucket_brigade* leftover_work = static_cast<apr_bucket_brigade*>(filter->ctx);
    if (leftover_work && !APR_BRIGADE_EMPTY(leftover_work)) {
        // Move the leftover bucket to our new brigade
        apr_bucket* leftover_bucket = APR_BRIGADE_FIRST(leftover_work);
        APR_BUCKET_REMOVE(leftover_bucket);
        APR_BRIGADE_INSERT_HEAD(bb, leftover_bucket);
        assert(APR_BRIGADE_EMPTY(leftover_work)); // There should only ever be zero or one buckets left over
    }

    Iterator beginning{bb, flush};
    Iterator end{EndIterator(bb)};

    /// Move buckets to a new brigade
    auto moveBuckets = [&](Iterator a, Iterator b, apr_bucket_brigade* dest) {
        // Split the last one 1st, as a split may invalidate all iterators after it
        b.split();
        a.split();
        apr_bucket* bucket = a.bucket();
        apr_bucket* last_bucket = b.bucket();
        // Return the char after what b was pointing at
        Iterator result{b};
        assert(b.bucket() == result.bucket()); // Should have same bucket here
        // Move all those buckets into the other brigade
        while (bucket != last_bucket) {
            apr_bucket* next = APR_BUCKET_NEXT(bucket);
            APR_BUCKET_REMOVE(bucket);
            APR_BRIGADE_INSERT_TAIL(dest, bucket);
            bucket = next;
        }
        return result;
    };

    // We could get a whole bunch of non-data buckets. The brigade would be technically not empty, but practically empty for our purposes
    if (beginning == end) {
        moveBuckets(beginning, end, completed_work);
        return flush();
    }

    // Called when we find a range of unchanged data
    RangeEvent<Iterator> onUnchangedData = [&](const Iterator &start,
                                               const Iterator &end) {
      // Move buckets from start up to the current into our completed_work
      // brigade
      return moveBuckets(start, end, completed_work);
    };

    // Called when new data to push out the filter arrives
    DataEvent newData = [&](const std::string& data) {
        // Create a new bucket to append to completed work
        // Copy the data to it. Needs to be copied because it's coming from a data dict, that will dissapear when the filter does.
        apr_bucket* bucket = apr_bucket_heap_create(data.c_str(), data.size(), NULL, filter->c->bucket_alloc);
        APR_BRIGADE_INSERT_TAIL(completed_work.brigade(), bucket);
    };

    // Log that we're gonna do some work
    const char* log_location = location.c_str();
    ap_log_rerror(APLOG_MARK, APLOG_DEBUG, APR_SUCCESS, filter->r, "Filtering Location: %s", log_location);

    // Get the server name and protocol
    const char* server_name = ap_get_server_name_for_url(filter->r);
    apr_port_t port = ap_get_server_port(filter->r);
    const char* protocol = ap_get_server_protocol(filter->r->server);
    std::stringstream hostname;
    hostname << protocol << "://" << server_name;
    if (!((strcmp(protocol, "https") == 0) && (port == 443)) &&
        !((strcmp(protocol, "http") == 0) && (port == 80)))
        hostname << ':' << port;

    // Is it CSS or HTML/XML ?
    bool isCSS(false);
    if ((filter->r) && (filter->r->content_type))
      isCSS = (strcmp(filter->r->content_type, "text/css") == 0);

    // Do the actual rewriting now: TODO: check the mime type for css/html
    Iterator tag_start =
        rewriteHTML(hostname.str(), location, *config, beginning, end,
                    onUnchangedData, newData, isCSS);

    // Store any left over data for next time
    if (tag_start != end) {
        if (!leftover_work)
            filter->ctx = leftover_work = static_cast<apr_bucket_brigade*>(apr_brigade_create(filter->r->pool, filter->c->bucket_alloc));
        moveBuckets(tag_start, end, leftover_work);
    }
    
    // Send all our comleted work to the next filter
    return flush();
}

}
}
