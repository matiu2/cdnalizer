#pragma once
/** Rewrites HTML so that resources in cloud files can be found
 */

#include "Config.hpp"

#include <string>
#include <memory>

namespace cdnalizer {

class Rewriter {
private:
    struct Impl;
    std::unique_ptr<Impl> impl;
public:
    Rewriter(const std::string& location,
             const Config& config);
};

/** Rewrite a flat HTML string, changing some urls to the CDN equivalents
 * The input must be complete, no chunking please.
 *
 * @param location the base location in the file system, or URL hierachy.
 *                 For expample if location is '/people' and we see a relative url like '<img src="images/a.gif" />'
 *                 we'll treat that url as /people/images/a.gif.
 * @param config   The configuration object to use
 * @param html     All the html content in one block, that we will be converting
 * @return         The new HTML string with the URLs re-written to the CDN equivalents
 */
std::string rewriteHTML(const std::string& location,
                        const Config& config,
                        const std::string& html);

/** 
 * */


}
