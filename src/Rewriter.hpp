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
    char // TODO: See how bucket brigade works


};

/**
 * Rewrites HTML, changing links and references to resources to point to they CDN equivalent
 *
 * @param location The location that this page is being served from. Knowing this helps us rewrite URLs with no '/' in front
 * @param config The configuration to use
 * @param html The html that we are re-writing
 * @return The newly re-written HTML
 */
std::string rewriteHTML(const std::string& location,
                        const Config& config,
                        const std::string& html);



}
