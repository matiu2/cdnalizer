#pragma once
/** Rewrites HTML so that resources in cloud files can be found
 */

#include "Config.hpp"

#include <string>
#include <memory>
#include <functional>

namespace cdnalizer {

using RangeEvent = std::function<void(const char*, const char*)>; /// Used for events where start and end pointers signify a range in the input
using DataEvent = std::function<void(std::string)>;               /// Used for events that generate new data
using EndEvent = std::function<void(const char*)>;

/** Rewrites links and references in HTML output to point to the CDN.
 *  For example /images/a.gif could become http://cdn.yoursite.com/images/a.gif
 *
 *  It doesn't do any output of its own; you just give it data, and it emits events.
 *  You can handle them how you like. That way, whatever output mechanism you use, you
 *  should be able to use it efficiently with little to no data copying.
 *
 * @param location the base location in the file system, or URL hierachy.
 *                 For expample if location is '/people' and we see a relative url like '<img src="images/a.gif" />'
 *                 we'll treat that url as /people/images/a.gif.
 * @param config   The configuration object to use
 * @param start    Pointer to the first character of the data to convert
 * @param end      One past the last character of the data (start + length)
 * @param noChange Event fired when we just got through a bunch of data, and we're not going to make a change to it.
 *                 Example usage: myRewriter.onNoChange = [](const char* a, const char* b) { passInputThroughToOutput(a, b); }
 *                 See whe rewriteHTML function for a more concrete example.
 * @param newData  Event fired when new data for the output stream has been generated
 * @param finished Event fired when we're unable to process any more data
 */
void rewriteHTML(const std::string& location, const Config& config,
                 const char* start, const char* end,
                 RangeEvent noChange, DataEvent newData, EndEvent finished);

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

}
