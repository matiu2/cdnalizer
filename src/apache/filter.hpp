#pragma once
/** Actually does the work of the filter */
#include "../Rewriter.hpp"
#include "../Config.hpp"
#include "utils.hpp"
#include "iterator.hpp"

extern "C" {
#include <apr_buckets.h>
#include <util_filter.h>
}

namespace cdnalizer {
namespace apache {

/** This function is the apache output filter.
 * It filters a bucket brigade (usually 8k of html content, broken into chunks (buckets), then
 * passes that on to the next filter.
 *
 * @param filter The filter chain (holds a bunch of state stuff)
 * @param bb The bucket brigade we're working on
 * @returns an apache status code
 */
apr_status_t filter(ap_filter_t *filter, apr_bucket_brigade *bb) {
    // Just pass on empty brigades
    if (APR_BRIGADE_EMPTY(bb)) { return APR_SUCCESS; }

    // TODO: Get our current path from Apache
    std::string location{"/"};

    // TODO: Load this from Apache
    Config config{{
        {"/images", "http://cdn.supa.ws/imgs"},
        {"/images2", "http://cdn.supa.ws/imgs2"},
        {"/aaa", "http://cdn.supa.ws/aaa"},
        {"/aab", "http://cdn.supa.ws/aab"},
        {"/aac", "http://cdn.supa.ws/aac"}
    }};

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
        // Get left over work from last time ?
        apr_bucket* leftover_bucket = APR_BRIGADE_FIRST(leftover_work);
        APR_BUCKET_REMOVE(leftover_bucket);
        APR_BRIGADE_INSERT_HEAD(bb, leftover_bucket);
        assert(APR_BRIGADE_EMPTY(leftover_work)); // There should only ever be zero or one buckets left over
    }
    using cdnalizer::apache::Iterator;
    using cdnalizer::apache::EndIterator;
    using cdnalizer::apache::BrigadeGuard;

    Iterator beginning{bb, flush};
    Iterator end{EndIterator(bb)};

    /// Move buckets to a new brigade
    auto moveBuckets = [&](Iterator a, Iterator b, apr_bucket_brigade* dest) {
        apr_bucket* bucket = a.split();
        apr_bucket* last_bucket = b.split();
        // Return the char after what b was pointing at
        Iterator result{b};
        assert(b.bucket() == result.bucket()); // Should have same bucket here
        // If it's not already the end of the world, we'll be returning the char after the last iterator
        if (b != end) {
            ++result;
            assert(last_bucket != result.bucket()); // result should have a new bucket here, because it's one after the split
        }
        apr_bucket* sentinel = APR_BUCKET_NEXT(last_bucket);
        // Move all those buckets into the other brigade
        while (bucket != sentinel) {
            apr_bucket* next = APR_BUCKET_NEXT(bucket);
            APR_BUCKET_REMOVE(bucket);
            APR_BRIGADE_INSERT_TAIL(dest, bucket);
            bucket = next;
        }
        return result;
    };

    // We could get a whole bunch of non-data buckets. The brigade would be technically not empty, but practically empty for our purposes
    if (beginning == end)
        return flush();

    // Called when we find a range of unchanged data
    auto onUnchangedData = [&](const Iterator& start, const Iterator& end) {
        // Move buckets from start up to the current into our completed_work brigade
        return moveBuckets(start, end, completed_work);
    };

    // Called when new data to push out the filter arrives
    auto newData = [&](const std::string& data) {
        // Create a new bucket to append to completed work
        // Copy the data to it. Needs to be copied because it's coming from a long lived data dict; not generated.
        apr_bucket* bucket = apr_bucket_heap_create(data.c_str(), data.size(), NULL, filter->c->bucket_alloc);
        APR_BRIGADE_INSERT_TAIL(completed_work.brigade(), bucket);
    };

    // Do the actual rewriting now
    Iterator tag_start = rewriteHTML<Iterator>(
        location, config, beginning, end, onUnchangedData, newData);

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
