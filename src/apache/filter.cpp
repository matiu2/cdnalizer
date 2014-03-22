#include "filter.hpp"

#include "../Rewriter.hpp"
#ifdef HAVE_CPP11
#include "../Rewriter_impl.hpp"
#else
#include "../Rewriter_impl_old.hpp"
#endif
#include "../Config.hpp"
#include "iterator.hpp"
#include "mod_cdnalizer.hpp"

extern "C" {

#include <apr_buckets.h>
#include <http_log.h>

#ifdef HAVE_APACHE_2_4
APLOG_USE_MODULE(cdnalizer_module);
#endif

}

namespace cdnalizer {
namespace apache {

#ifndef HAVE_CPP11
struct Flusher {
    ap_filter_t* filter;
    BrigadeGuard& completed_work;
    apr_status_t operator ()() {
        apr_status_t result = ap_pass_brigade(filter->next, completed_work);
        apr_brigade_cleanup(completed_work);
        return result;
    }
    Iterator operator()(const Iterator& start, const Iterator& end) {
        // Move buckets from start up to the current into our completed_work brigade
        return moveBuckets(start, end, completed_work);
    }
    static Iterator moveBuckets(Iterator a, Iterator b, apr_bucket_brigade* dest) {
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
    }
    /// Called when new data to push out the filter arrives
    void operator ()(const std::string& data) {
        // Create a new bucket to append to completed work
        // Copy the data to it. Needs to be copied because it's coming from a data dict, that will dissapear when the filter does.
        apr_bucket* bucket = apr_bucket_heap_create(data.c_str(), data.size(), NULL, filter->c->bucket_alloc);
        APR_BRIGADE_INSERT_TAIL(completed_work.brigade(), bucket);
    };
};

#endif

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
    #ifdef HAVE_CPP11
    auto flush = [&]() {
        apr_status_t result = ap_pass_brigade(filter->next, completed_work);
        apr_brigade_cleanup(completed_work);
        return result;
    };
    #else
    Flusher flusher{filter, completed_work};
    #endif

    // See if there's any work left over from last time
    apr_bucket_brigade* leftover_work = static_cast<apr_bucket_brigade*>(filter->ctx);
    if (leftover_work && !APR_BRIGADE_EMPTY(leftover_work)) {
        // Get left over work from last time ?
        apr_bucket* leftover_bucket = APR_BRIGADE_FIRST(leftover_work);
        APR_BUCKET_REMOVE(leftover_bucket);
        APR_BRIGADE_INSERT_HEAD(bb, leftover_bucket);
        assert(APR_BRIGADE_EMPTY(leftover_work)); // There should only ever be zero or one buckets left over
    }

    Iterator beginning{bb, flusher};
    Iterator end{EndIterator(bb)};

    /// Move buckets to a new brigade
    #ifdef HAVE_CPP11
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
    #endif

    // We could get a whole bunch of non-data buckets. The brigade would be technically not empty, but practically empty for our purposes
    if (beginning == end) {
        flusher.moveBuckets(beginning, end, completed_work);
        return flusher();
    }

    // Called when we find a range of unchanged data
    #ifdef HAVE_CPP11
    std::function<Iterator(const Iterator&, const Iterator&)> onUnchangedData = [&](const Iterator& start, const Iterator& end) {
        // Move buckets from start up to the current into our completed_work brigade
        return flusher.moveBuckets(start, end, completed_work);
    };
    #else
    std::function<Iterator(const Iterator&, const Iterator&)> onUnchangedData=flusher;
    #endif

    // Called when new data to push out the filter arrives
    #ifdef HAVE_CPP11
    auto newData = [&](const std::string& data) {
        // Create a new bucket to append to completed work
        // Copy the data to it. Needs to be copied because it's coming from a data dict, that will dissapear when the filter does.
        apr_bucket* bucket = apr_bucket_heap_create(data.c_str(), data.size(), NULL, filter->c->bucket_alloc);
        APR_BRIGADE_INSERT_TAIL(completed_work.brigade(), bucket);
    };
    #else
    std::function<void(const std::string&)> newData = flusher;
    #endif

    // Log that we're gonna do some work
    const char* log_location = location.c_str();
    ap_log_rerror(APLOG_MARK, APLOG_DEBUG, APR_SUCCESS, filter->r, "Filtering Location: %s", log_location);

    // Do the actual rewriting now
    #ifdef HAVE_CPP
    Iterator tag_start = rewriteHTML(location, *config, beginning, end, onUnchangedData, newData);
    #else
    Iterator tag_start = rewriteHTML(location, *config, beginning, end, flusher, flusher);
    #endif

    // Store any left over data for next time
    if (tag_start != end) {
        if (!leftover_work)
            filter->ctx = leftover_work = static_cast<apr_bucket_brigade*>(apr_brigade_create(filter->r->pool, filter->c->bucket_alloc));
        flusher.moveBuckets(tag_start, end, leftover_work);
    }
    
    // Send all our comleted work to the next filter
    return flusher();
}

}
}
