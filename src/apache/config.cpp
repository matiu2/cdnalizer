/**
 * © Copyright 2014 Matthew Sherborne. All Rights Reserved.
 * License: Apache License, Version 2.0 (See LICENSE.txt)
 **/
#include "config.hpp"
#include "../Config.hpp"
#include "mod_cdnalizer.hpp"

APLOG_USE_MODULE(cdnalizer_module);

// Our C style parts for Apache registration
extern "C" {
#include <http_log.h>

using cdnalizer::Config;

/// Delete a config object from a pool that's dying
apr_status_t deleteConfig(void* memory) {
    Config* cfg = static_cast<Config*>(memory);
    cfg->~Config();
    return APR_SUCCESS;
}

/// Create a config object for a dir
void* cdnalizer_create_dir_config(apr_pool_t* pool, char* context) {
    void* memory = apr_palloc(pool, sizeof(Config));
    Config* cfg;
    if (context)
        cfg = new (memory) Config({}, context);
    else
        cfg = new (memory) Config();
    apr_pool_cleanup_register(pool, memory, &deleteConfig, &deleteConfig);
    return cfg;
}

/// Add one config to another
void* cdnalizer_merge_dir_configs(apr_pool_t* pool, void* base, void* add) {
    Config* cfg1 = static_cast<Config*>(base);
    Config* cfg2 = static_cast<Config*>(add);
    // Make the result
    void* memory = apr_palloc(pool, sizeof(Config));
    Config* result = new (memory) Config(*cfg1);
    apr_pool_cleanup_register(pool, memory, &deleteConfig, &deleteConfig);
    *result += *cfg2;
    return result;
}

/// Reads a line from the Apache config
const char *addCDNPath(cmd_parms *cmd, void *memory, const char *arg1, const char* arg2) {
    ap_log_error(APLOG_MARK, APLOG_DEBUG, APR_SUCCESS, cmd->server, "Reading CDN->url pair: %s->%s", arg1, arg2);
    Config* cfg = static_cast<Config*>(memory);
    cfg->addPath(arg1, arg2);
    return NULL;
}

}
