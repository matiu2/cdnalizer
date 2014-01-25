/* 
**  mod_cdnalizer.c -- Apache sample cdnalizer module
**  [Autogenerated via ``apxs -n cdnalizer -g'']
**
**  To play with this sample module first compile it into a
**  DSO file and install it into Apache's modules directory 
**  by running:
**
**    $ apxs -c -i mod_cdnalizer.c
**
**  Then activate it in Apache's apache2.conf file for instance
**  for the URL /cdnalizer in as follows:
**
**    #   apache2.conf
**    LoadModule cdnalizer_module modules/mod_cdnalizer.so
**    <Location /cdnalizer>
**    SetHandler cdnalizer
**    </Location>
**
**  Then after restarting Apache via
**
**    $ apachectl restart
**
**  you immediately can request the URL /cdnalizer and watch for the
**  output of this module. This can be achieved for instance via:
**
**    $ lynx -mime_header http://localhost/cdnalizer 
**
**  The output should be similar to the following one:
**
**    HTTP/1.1 200 OK
**    Date: Tue, 31 Mar 1998 14:42:22 GMT
**    Server: Apache/1.3.4 (Unix)
**    Connection: close
**    Content-Type: text/html
**  
**    The sample page from mod_cdnalizer.c
*/ 

#include "mod_cdnalizer.hpp"
#include "../Rewriter_impl.hpp"
#include "filter.hpp"
#include "config.hpp"

extern "C" {
#include <apache2/httpd.h>
#include <apache2/http_protocol.h>

static const char cdnalizer_filter_name[] = "CDNALIZER";

/*
static const command_rec cdnalizer_filter_cmds[] = {
    AP_INIT_RAW_ARGS("CDNRewriteRule", cmd_cdnrewriterule, NULL, OR_FILEINFO,
                     "an URL-applied regexp-pattern and a substitution (cdn) URL");
}
*/

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
    //cdnalizer_create_dir_config,     /* create per-dir    config structures */
    //cdnalizer_merge_dir_configs,     /* merge  per-dir    config structures */
    NULL,
    NULL,
    NULL,                  /* create per-server config structures */
    NULL,                  /* merge  per-server config structures */
    cdnalizer_config_directives, /* table of config file directives       */
    cdnalizer_register_hooks  /* register hooks                      */
};

AP_DECLARE_MODULE(cdnalizer_module);

}

namespace cdnalizer {
namespace apache {

Iterator rewriteHTML(const std::string& location, const Config& config,
                 Iterator start, Iterator end,
                 RangeEvent<Iterator> noChange, DataEvent newData);

}
}
