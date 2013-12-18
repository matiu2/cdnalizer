/** Actually does the work of the filter */
#include "../Rewriter.hpp"
#include "../Config.hpp"
#include "utils.hpp"

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
static apr_status_t filter(ap_filter_t *filter, apr_bucket_brigade *bb) {
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

    // Make a fresh, RAII protected Apache Bucket Brigade
    auto makeBB = [&]() {
        return BrigadeGuard(filter->r->pool, filter->c->bucket_alloc);
    };

    // Place to store any left over work
    apr_bucket_brigade* leftover_work = static_cast<apr_bucket_brigade*>(filter->ctx);
    if (leftover_work == NULL) {
        // This is the 1st time we've seen this request. Make a place to store any left over data
        filter->ctx = leftover_work = static_cast<apr_bucket_brigade*>(apr_brigade_create(filter->r->pool, filter->c->bucket_alloc));
    }  else if (!APR_BRIGADE_EMPTY(leftover_work)) {
        // Get left over work from last time ?
        apr_bucket* leftovers = APR_BRIGADE_FIRST(leftover_work);
        APR_BUCKET_REMOVE(leftovers);
        APR_BRIGADE_INSERT_HEAD(bb, leftovers);
        assert(APR_BRIGADE_EMPTY(leftover_work)); // There should only ever be zero or one buckets left over
    }

    // Work to be sent to the next filter on flush or ending
    BrigadeGuard completed_work = makeBB();

    // Shared vars
    apr_bucket *bucket;   // The current bucket
    const char *data;     // The current data block
    apr_size_t length;    // The length of the current data block

    // Custom exceptions
    struct NoMoreBuckets {}; // Thrown when we hit the last bucket
    struct EndOfRequest {};  // Thrown when we have eaten all the data in the request

    // Sub Functions that use the shared vars //

    auto flush = [&]() {
        /// Send our completed_work to the next filter
        checkStatusCode(ap_pass_brigade(filter, completed_work));
        completed_work = makeBB();
    };

    auto save = [&]() {
        /// Move the current bucket into our saved_work stream
        APR_BUCKET_REMOVE(bucket);
        APR_BRIGADE_INSERT_TAIL((apr_bucket_brigade*)completed_work, bucket);
    };

    auto next = [&]() {
        /// Get the next data bucket. Fill in 'data' and 'length'
        start:
            // As we move completed work out of bb, we always get the first bucket
            bucket = APR_BRIGADE_FIRST(bb);
            // Did we hit one past the end ?
            if (bucket == APR_BRIGADE_SENTINEL(bb))
                throw NoMoreBuckets();
            // Is this the end of the request ?
            if (APR_BUCKET_IS_EOS(bucket))
                throw EndOfRequest();
            // Do we need to flush
            if (APR_BUCKET_IS_FLUSH(bucket)) {
                flush();
                goto start;
            }
            // Ignore metadata buckets
            if (APR_BUCKET_IS_METADATA(bucket)) {
                save();
                goto start;
            }
        // Read the data
        checkStatusCode(apr_bucket_read(bucket, &data, &length, APR_BLOCK_READ));
    };

    // Event Handlers //////////////////////

    // Called when we find a range of unchanged data
    auto onUnchangedData = [&](const char* start, const char* end) {
        // Move work to completed work
        assert(start == data);
        // Split the bucket if we need to
        if (end != data+length)
            apr_bucket_split(bucket, end-data);
        // Send the bucket to 'done_work'
        save();
        next();
        return data; // Continue at the start of the new bucket
    };

    // Called when new data arrives (new html attribute value parts)
    auto onNewData = [&](const std::string& data) {
        // 'data' lives as long as our config which will certainly be longer than the bucket brigade
        apr_bucket* newBucket = apr_bucket_immortal_create(data.c_str(), data.length(), filter->c->bucket_alloc);
        APR_BRIGADE_INSERT_TAIL((apr_bucket_brigade*)completed_work, newBucket);
    };

    // Main processing loop
    try {
        while (1) {
            next(); // Get the next bucket of data
            // Filter it
            const char* tag_start = rewriteHTML<const char*>(
                location, config, data, data+length, onUnchangedData, onNewData);
            // The algorithm should always push a 'onChange' event at the end, that will nullify
            // the current bucket, and get the next automatically.
            // If there is no next bucket, it'll throw and this line will never be hit
            // So there should be one bucket left at this point, and it'll contain the
            // start of tag that has no ending.
            assert(tag_start == data);
            // tag_start is either the start of a tag that we couldn't find an ending to,
            // or the actual end of this bucket's data
            // Put the leftovers in a bucket for later
            APR_BUCKET_REMOVE(bucket);
            APR_BRIGADE_INSERT_TAIL((apr_bucket_brigade*)completed_work, bucket);
        }
    } catch(EndOfRequest) {
        // There is no more data and we won't be called again
        // Flush our leftovers
        flush();
        if (!APR_BRIGADE_EMPTY(leftover_work)) {
            checkStatusCode(ap_pass_brigade(filter, completed_work));
        }
        // Kill the context and everthing
        checkStatusCode(apr_brigade_destroy(leftover_work));
    } catch(NoMoreBuckets) {
        // No more data in this brigade. But we'll be called again
        flush();
    }
    return APR_SUCCESS;
}

}
}
