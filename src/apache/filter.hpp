#pragma once
/** Actually does the work of the filter */

extern "C" {
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
apr_status_t filter(ap_filter_t *filter, apr_bucket_brigade *bb);

}
}
