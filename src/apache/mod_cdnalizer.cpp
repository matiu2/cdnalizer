/**
 * © Copyright 2014 Matthew Sherborne. All Rights Reserved.
 * License: Apache License, Version 2.0 (See LICENSE.txt)
 **/

#include "mod_cdnalizer.hpp"
#include "filter.hpp"
#include "config.hpp"

extern "C" {
#include <httpd.h>
#include <http_protocol.h>

static const char cdnalizer_filter_name[] = "CDNALIZER";

static apr_status_t cdnalize_out_filter(ap_filter_t *filter, apr_bucket_brigade *bb)
{
    try {
        return cdnalizer::apache::filter(filter, bb);
    } catch (...) {
        return APR_OS_START_USERERR;
    }
}

static void cdnalizer_register_hooks(apr_pool_t *)
{
    ap_register_output_filter(cdnalizer_filter_name, cdnalize_out_filter, NULL,
                              AP_FTYPE_CONTENT_SET);
}


/* Dispatch list for API hooks */
module AP_MODULE_DECLARE_DATA cdnalizer_module = {
    STANDARD20_MODULE_STUFF, 
    cdnalizer_create_dir_config,     /* create per-dir    config structures */
    cdnalizer_merge_dir_configs,     /* merge  per-dir    config structures */
    NULL,                  /* create per-server config structures */
    NULL,                  /* merge  per-server config structures */
    cdnalizer_config_directives, /* table of config file directives       */
    cdnalizer_register_hooks  /* register hooks                      */
};

AP_DECLARE_MODULE(cdnalizer_module);

}
