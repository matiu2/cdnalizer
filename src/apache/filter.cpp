/**
 * © Copyright 2014 Matthew Sherborne. All Rights Reserved.
 * License: Apache License, Version 2.0 (See LICENSE.txt)
 **/
#include "filter.hpp"

#include "../Rewriter.hpp"
#include "../Config.hpp"
#include "../Parser.hpp"
#include "BucketWrapper.hpp"
#include "BucketEvent.hpp"
#include "contentTypes.hpp"
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

/// Move buckets to a new brigade
void moveBuckets(BucketWrapper &a, BucketWrapper &b, apr_bucket_brigade *dest) {
  // Split the last one 1st, as a split may invalidate all iterators after it
  apr_bucket *bucket = a.bucket();
  apr_bucket *last_bucket = b.bucket();
  // Move all those buckets into the new brigade
  while (bucket != last_bucket) {
    apr_bucket *next = APR_BUCKET_NEXT(bucket);
    APR_BUCKET_REMOVE(bucket);
    APR_BRIGADE_INSERT_TAIL(dest, bucket);
    bucket = next;
  }
}

/// Called when we need to flush our completed work
apr_status_t flush(BrigadeGuard& completed_work) {
  apr_status_t result = ap_pass_brigade(filter->next, completed_work);
  apr_brigade_cleanup(completed_work);
  return result;
}

/// This is our status that we store between calls to filter
struct Status {
  apr_bucket_brigade* leftover_work = nullptr;
  std::unique_ptr<cdnalizer::Parser> p;
};

apr_status_t filter(ap_filter_t *filter, apr_bucket_brigade *bb) {
  // Just pass on empty brigade if it is empty
  if (APR_BRIGADE_EMPTY(bb)) {
    return APR_SUCCESS; }

  // Get our current path from Apache
  std::string location{filter->r->uri};
  auto pos = location.rfind('/');
  if (pos != std::string::npos)
    location.resize(pos + 1);

  // Get our config
  Config *config = static_cast<Config *>(
      ap_get_module_config(filter->r->per_dir_config, &cdnalizer_module));

  // Get the server name and protocol
  const char *server_name = ap_get_server_name_for_url(filter->r);
  apr_port_t port = ap_get_server_port(filter->r);
  const char *protocol = ap_get_server_protocol(filter->r->server);
  std::stringstream hostname;
  hostname << protocol << "://" << server_name;
  if (!((strcmp(protocol, "https") == 0) && (port == 443)) &&
      !((strcmp(protocol, "http") == 0) && (port == 80)))
    hostname << ':' << port;

  // This is the work to be sent to the next filter on flush or ending
  BrigadeGuard completed_work{filter->r->pool, filter->c->bucket_alloc};

  // See if there's any work left over from last time
  apr_bucket_brigade *leftover_work =
      static_cast<apr_bucket_brigade *>(filter->ctx);
  if (leftover_work && !APR_BRIGADE_EMPTY(leftover_work)) {
    // Move the leftover bucket to our new brigade
    // There should only ever be zero or one buckets left over from last time
    apr_bucket *leftover_bucket = APR_BRIGADE_FIRST(leftover_work);
    APR_BUCKET_REMOVE(leftover_bucket);
    APR_BRIGADE_INSERT_HEAD(bb, leftover_bucket);
    assert(APR_BRIGADE_EMPTY(leftover_work));
  }

  BucketWrapper bucket{bb, [&completed_work]() { flush(completed_work); }};
  BucketWrapper end(lastBucket(bb));

  // We could get a whole bunch of non-data buckets. The brigade would be
  // technically not empty, but practically empty for our purposes
  if (bucket == end) {
    moveBuckets(bucket, end, completed_work);
    return flush(completed_work);
  }

  // Find the content type of what we're serving
  // TODO: detect all content types here
  ContentType ct;
  if ((filter->r) && (filter->r->content_type))
    if (strcmp(filter->r->content_type, "text/css") == 0)
      ct = CSS;

  // TODO: Make it parse other content types
  cdnalizer::parser::Parser parser(ct);

  // Log that we're gonna do some work
  const char *log_location = location.c_str();
  ap_log_rerror(APLOG_MARK, APLOG_DEBUG, APR_SUCCESS, filter->r,
                "Filtering Location: %s", log_location);

  // Unchanged work
  BucketWrapper unchanged_data = bucket;

  while (bucket != end) {
    BucketEvent result = parser.parseNextBlock(bucket.begin(), bucket.end());
    if (result.splitPoint != nullptr)
      bucket.split(result.splitPoint);
    // Move all the work we've done so far to completed_work
    auto next = bucket.next();
    moveBuckets(unchanged_data, bucket, completed_work);
    // Remember where we are for future 
    bucket = unchanged_data = next;
    if (!result.newData.empty()) {
      // Create a new bucket to append to completed work
      // Copy the data to it. Needs to be copied because it's coming from a data
      // dict, that will dissapear when the filter does.
      apr_bucket *newDataBucket =
          apr_bucket_heap_create(result.newData.c_str(), result.newData.size(),
                                 NULL, filter->c->bucket_alloc);
      APR_BRIGADE_INSERT_TAIL(completed_work.brigade(), newDataBucket);
    }
  }

  // Push the last buckets that we're not doing anything with to our completed_work brigade
  if (unchanged_data != end)
    moveBuckets(unchanged_data, end, completed_work);

  // Send all our comleted work to the next filter
  return ap_pass_brigade(completed_work);
}

}
}
