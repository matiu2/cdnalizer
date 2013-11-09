#pragma once

#include "../Config.hpp"

#include <istream>
#include <ostream>

namespace cdnalizer {
namespace stream {

/** Rewrite a flat HTML string, changing some urls to the CDN equivalents
 * The input must be complete, no chunking please.
 *
 * @param location the base location in the file system, or URL hierachy.
 *                 For expample if location is '/people' and we see a relative url like '<img src="images/a.gif" />'
 *                 we'll treat that url as /people/images/a.gif.
 * @param config   The configuration object to use
 * @param html     All the html content in one block that we will be converting
 * @param output   The output stream that new html will be written to
 */
void rewriteHTML(const std::string& location,
                 const Config& config,
                 std::istream& html,
                 std::ostream& output);

}
}
