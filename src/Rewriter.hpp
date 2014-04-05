#pragma once
/** Rewrites HTML so that resources in cloud files can be found
 *
 * © Copyright 2014 Matthew Sherborne. All Rights Reserved.
 * License: Apache License, Version 2.0 (See LICENSE.txt)
 */

#include "Config.hpp"

#include <string>
#include <memory>
#include <functional>

namespace cdnalizer {

/** Rewrites links and references in HTML output to point to the CDN.
 *  For example /images/a.gif could become http://cdn.yoursite.com/images/a.gif
 *
 *  It doesn't do any output of its own; you just give it data, and it emits events.
 *  You can handle them how you like. That way, whatever output mechanism you use, you
 *  should be able to use it efficiently with little to no data copying.
 *
 * @param server_url
 *                 The server_url will start with http:// eg. http://www.supa.ws
 *                 and any urls we find starting with that, will be normalized down, eg. http://www.supa.ws/images/x.gif would
 *                 become /images/x.gif for the sake of searching
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
 * @returns The place where we reading when we hit @a end - at the time of writing
 *          if we were in the middle of a tag, we'll return the position of the '<',
 *          otherwise, it'll be the same as end.
 */
/// Used for events that generate new data
template <typename iterator, typename char_type, typename RangeEvent, typename DataEvent>
iterator rewriteHTML(const std::string& server_url, const std::string& location, const Config& config,
                 iterator start, iterator end,
                 RangeEvent noChange, DataEvent newData);

}
