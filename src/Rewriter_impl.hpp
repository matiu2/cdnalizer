/// A C++03 implementation of Rewriter
#pragma once
#include "Rewriter.hpp"

#include "Config.hpp"
#include "utils.hpp"

#include <algorithm>
#include <iterator>   // find search
#include <cctype>     // tolower

namespace cdnalizer {

template <typename iterator, typename char_type, typename RangeEvent, typename DataEvent>
struct Rewriter {
    /// An exception we throw when we have finished parsing the input, and catch in the main loop.
    struct Done {
        iterator pos;  /// The character that the next run should start with. (One past the last character we read).
        Done() {}
        Done(iterator pos) : pos(pos) {}
    }; 
    struct NotFound{}; /// Exception for if we can't find the attribute
    typedef cdnalizer::pair<iterator> pair;

    const std::string& server_url;
    const std::string& location;
    const Config& config;
    iterator start;
    iterator end;
    RangeEvent&noChange;
    DataEvent newData;

    iterator nextNoChangeStart;

    Rewriter(const std::string& server_url, const std::string& location, const Config& config,
             iterator start, iterator end,
             RangeEvent& noChange, DataEvent& newData) 
    : server_url(server_url), location(location), config(config), start(start), end(end), noChange(noChange), newData(newData), nextNoChangeStart(start) {}

    /** Given the start of an HTML <tag> it find the end of it.
     * @return The end of the tag, or the end of the input if there was no tag end */
    iterator findTagEnd(iterator start) {
        while (start != end) {
            if (*start == '<')
                break;
            if (*start == '>')
                break;
            ++start;
        }
        return start;
    }
    /// Takes the start and end of a tag and retuns the tag name.
    pair getTagName(const pair& tag) {
        const std::string ws=" \t\r\n";
        iterator name_start = tag.first;
        iterator name_end = std::find_first_of(++name_start, tag.second, ws.begin(), ws.end());
        // Return the result
        return pair(name_start, name_end);
    }
    static bool isWS(char c) {
        switch (c) {
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                return true;
            default:
                return false;
        }
    }
    static bool isNotWS(char c) { return !isWS(c); }
    iterator find(iterator start, char c, iterator tag_end) {
        iterator res = std::find(start, tag_end, c);
        if (res == tag_end)
            throw NotFound();
        return res;
    };
    /// Case insensitive compare
    static bool lower_compare(char_type a, char_type b) {
        return std::tolower(a) == std::tolower(b);
    };
    /** Returns the start and end of the attribute value that you want
     * @param tag The range that of the tag, within which we'll search
     * @param attrib_name The attribute name that we're searching for. Must be all lower case.
     * @return the start and end of the attribute value, or {tag.second, tag.second} if the attribute is not found
     */
    pair findAttribute(const pair& tag, const std::string& attrib_name) {
        const std::string quotes="\"'";

        iterator tag_end = tag.second;
        iterator pos = tag.first;

        while (pos != tag_end) {
            try {
                // Find ' ' .. tagName .. '=' .. '"' .. tagValue .. '"'
                iterator space = find(pos, ' ', tag_end);
                iterator after_space = space;
                ++after_space;
                iterator equals = find(space, '=', tag_end);
                iterator quote1 = std::find_first_of(equals, tag_end, quotes.begin(), quotes.end());
                iterator after_quote = quote1;
                iterator quote2 = find(++after_quote, *quote1, tag_end);
                // Get ready for the next run
                pos = quote2; 
                ++pos;
                // Check if this is the attribute we care about
                // There must be an attribute name (' ' .. 'attrib_name' .. '=') not (" =")
                // Skip over multiple adjacent whitespace before the attrib name
                iterator attrib_name_start = std::find_if(after_space, equals, isNotWS);
                iterator attrib_name_end = std::find_if(attrib_name_start, equals, isWS);
                pair attrib_name_range(attrib_name_start, attrib_name_end);
                // Check if the attribute name is the one we're looking for
                // ‘equal(Iterator<Flusher>&, Iterator<Flusher>&, __normal_iterator<const char*, string>, __normal_iterator<const char*, string>, bool (&)(char, char))’
                if (utils::equal(attrib_name_start, attrib_name_end, attrib_name.begin(), attrib_name.end(), &lower_compare))
                    return pair(++quote1, quote2);
            } catch (NotFound) {
                // We couldn't find any more attributes in this tag
                break;
            }
        }
        return pair(tag_end, tag_end);
    }
    /** Returns true if 'path' is relative.
     *
     * ie. if it doesn't start with '/', 'http://', or 'https://'
     *
     * @param path We'll check if this is relative, or ablosute
     * @return true of 'path' is relative or empty
     **/
    static bool is_relative(const pair& path) {
        return utils::is_relative(path.first, path.second);
    }
    /** Takes the start and end of the attribute value that we care about and emits events, possibly changing the value.
     *
     * If we don't have anything to change, just outputs nothing and leaves nextNoChangeStart unchanged
     *
     * @param attrib_range The range in the input, of the attribute value we care about
     * @returns true if all iterators after attrib start need recalculating
     *
     */
    bool handleAttributeValue(const pair& attrib_range) {
        // Work out the attrib value we'll search the config DB for
        std::string attrib_value;
        std::back_insert_iterator<std::string> value_putter = std::back_inserter(attrib_value);

        long added_chars = 0; // The amount of chars we add, to make attrib_value look up in the DB
        iterator copying_start = attrib_range.first; // Where we start copying the attribute from for search
        bool included_server_url = false;

        if (is_relative(attrib_range)) {   // eg. attrib_range='images/a.gif'
            // Prepend the attrib value with our current location if it's a relative path
            std::copy(location.begin(), location.end(), value_putter);  // eg. location='/blog'
            added_chars = location.length();
            if (*location.rbegin() != '/') {
                *value_putter++ = '/';
                ++added_chars;
            } // eg. attrib_value='/blog/'
        } else {
            // See if we need to strip off the server_url in order to search better
            if (!server_url.empty()) {
                std::pair<std::string::const_iterator, iterator> match = utils::mismatch(server_url.begin(), server_url.end(), attrib_range.first, attrib_range.second);
                if (match.first == server_url.end()) {
                    copying_start = match.second;
                    included_server_url = true;
                }
            }
        }
        
        // Now copy the bit we need to use for searching from the attribute
        std::copy(copying_start, attrib_range.second, value_putter);

        // eg. attrib_value='/blog/images/a.gif'
        // eg. added_chars = 6 = len('/blog/')

        // See if we have a replacement, if we search for /images/abc.gif .. we'll get the CDN for /images/ (if that's in the config)
        // 'found' will be like {"/images/", "http://cdn.supa.ws/images/"}
        try {
            Config::CDNRefPair found = config.findCDNUrl(attrib_value);
            const std::string& base_path=found.first;
            const std::string& cdn_url=found.second;
            // Check if the path we found is a substring of the value
            if (attrib_value.length() > base_path.length()) {
                std::pair<std::string::const_iterator, std::string::const_iterator> match = std::mismatch(base_path.begin(), base_path.end(), attrib_value.begin());
                if (match.first == base_path.end()) {
                    // If the attribute value starts with the base_path, replace the bit after with the new cdn url
                    std::string result;
                    result.reserve(cdn_url.length() + attrib_value.length() - base_path.length());
                    std::back_insert_iterator<std::string> out = back_inserter(result);
                    std::copy(cdn_url.begin(), cdn_url.end(), out);
                    // Push it out
                    // Make sure we got given actual event handlers
                    // Output the unchanged bits
                    // Next time the 'unchanged data' will start with the part of the attrib value after the base_path (that has been replaced)
                    nextNoChangeStart = noChange(nextNoChangeStart, attrib_range.first);
                    // At this point **all iterators** apart from nextNoChangeStart may be **corrupt and useless** and should not be used.
                    // This is because the noChange callback may drop any old data before nextNoChangeStart
                    // Send on the new data
                    newData(cdn_url);
                    // Move the nextNoChangeStart on to the end of the value
                    size_t increment = base_path.length();
                    if (included_server_url)
                        increment += server_url.length();
                    increment -= added_chars;
                    for(size_t i=0; i < increment; ++i)
                        ++nextNoChangeStart;
                    return true;
                }        
            }
        } catch (Config::NotFound) {}
        return false; // We can't replace that path, just carry on..
    };
    /** Once we've found a tag, handles checking, then possibly emiting events to change the attribute.
     *
     * If we change one of the attributes, emit the 'noChange' event for the data before the attribute value,
     * then the 'newData' event for the new attribute value, then set the @a nextNoChangeStart iterator to the '"' after
     * the attribute value.
     *
     * @param tag The tag that we found
     * @param nextNoChangeStart the start of the next block of unchanged data
     * @returns the new position for the algorithm to carry on searching from. Generally the end of the tag.
     *          We need to return it because if we split an iterator, all others around and after it may be invalidated.
     *
     * */
    iterator handleTag(const pair& tag) {
        iterator tag_end = tag.second;
        // Get the tag name
        pair tag_name = getTagName(tag);
        if (!tag_name) 
            return tag_end;
        // See if we care about it
        const std::string& attrib_name = config.getAttrib(tag_name);
        if (attrib_name.empty())
            return tag_end;
        // If we care, get the attribute value
        pair attrib = findAttribute(tag, attrib_name);
        if (attrib.first == tag.second)
            return tag_end;
        // If we found the attribute we wanted, see if we can get a new value from the config
        bool needRecalc = handleAttributeValue(attrib);
        if (needRecalc)
            // The old tag_end may have been invalidated by the noChange event (eg. by apache splitting the attrib iterator, invalidating everything after it)
            // So we'll search for the tag_end again
            tag_end = findTagEnd(nextNoChangeStart);
        return tag_end;
    }
    iterator run() {
        // The meat of the algorithm
        iterator pos = start;
        try {
            while (pos != end) {
                // Find the next tag
                iterator tag_start = std::find(pos, end, '<');
                if (tag_start == end)
                    // There are no more tags, send all the data
                    throw Done(end);
                iterator next = tag_start;
                if (++next == end)
                    // The tag is just before the end of the data, send everything before it
                    throw Done(tag_start);
                if (*next == '/') {
                    // This is a closing tag, continue the search
                    pos = next;
                    continue;
                }
                iterator tag_end = findTagEnd(next);
                if (tag_end == end)
                    // We don't have the end of the tag, copy up to the start of it and break
                    throw Done(tag_start);
                // We found a tag
                pair tag(tag_start, tag_end);
                pos = handleTag(tag);
            }
        } catch (Done e) {
            // We can push out the nuchanged data now
            return noChange(nextNoChangeStart, e.pos);
        }
        return end;
    }
};

template <typename iterator, typename char_type, typename RangeEvent, typename DataEvent>
iterator rewriteHTML(const std::string& server_url, const std::string& location, const Config& config,
                     iterator start, iterator end,
                     RangeEvent noChange, DataEvent newData) 
{
    Rewriter<iterator, char_type, RangeEvent, DataEvent> rewriter(server_url, location, config, start, end, noChange, newData);
    return rewriter.run();
}

}
