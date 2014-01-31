#pragma once
/** Loads the Apache config into our generic config object.
 *
 *  These functions are registered with Apach in mod_cdnalizer.cpp
 **/

// Our C style parts for Apache registration
extern "C" {

#include <httpd.h>
#include <http_config.h>

// Create a config object for a directory context
void* cdnalizer_create_dir_config(apr_pool_t* pool, char* context);

/// Add one config to another
void* cdnalizer_merge_dir_configs(apr_pool_t* pool, void* base, void* add);

// Reads a line from the Apache config
const char *addCDNPath(cmd_parms *cmd, void *cfg, const char *arg1, const char* arg2);

// List of Directives
static const command_rec cdnalizer_config_directives[] = {
    AP_INIT_ITERATE2(
        "CDN_URL", addCDNPath, NULL, OR_OPTIONS,
        "A map of 'path found' to 'cdn url', eg /images http://cdn.supa.ws/imgs"),
    // TODO: DEL_CDN_URL
    /*
    AP_INIT_ITERATE(
        "DEL_CDN_URL", delCDNPath, NULL, OR_OPTIONS,
        "Remove a CDN_URL pair (by the url/key)"),
    */
    {nullptr,nullptr,nullptr,0,RAW_ARGS,nullptr} // Leave this here, or you get segfaults matey
};


}

