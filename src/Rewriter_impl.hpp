#include "Rewriter.hpp"

#include "Config.hpp"

#include <algorithm>
#include <iterator>   // find search
#include <cctype>     // tolower

namespace cdnalizer {

struct Rewriter {
    /// An exception we throw when we have finished parsing the input, and catch in the main loop.
    struct Done {
        iterator pos;  /// The character that the next run should start with. (One past the last character we read).
    }; 

    const std::string& location;
    const Config& config;
    iterator start;
    iterator end;
    
    // Events
    RangeEvent noChange;
    DataEvent newData;

    const char* ws=" \t\r\n";
    const char* ws_end=ws+4;

    Rewriter(
        const std::string& location,
        const Config& config,
        iterator start,
        iterator end,
        
        // Events
        RangeEvent noChange,
        DataEvent newData
    ) : location{location}, config{config}, start{start}, end{end},
        noChange{noChange}, newData{newData} {}

    /** Run the conversion.
     * @returns The pointer to the last bit of the data that we actually use
     */
    iterator run() {
        iterator pos = start;
        iterator copy_from = start;
        try {
            while (pos != end) {
                // Find the next tag
                auto tag_start = std::find(pos, end, '<');
                if (tag_start == end)
                    // There are no more tags, send all the data
                    throw Done{end};
                if (tag_start+1 == end)
                    // The tag is just before the end of the data, send everything before it
                    throw Done{tag_start};
                if (tag_start[1] == '/') {
                    // This is a closing tag, continue the search
                    pos = tag_start+1;
                    continue;
                }
                const std::string tag_ends{"<>"};
                auto tag_end = std::find_first_of(tag_start, end, tag_ends.begin(), tag_ends.end());
                if (tag_end == end)
                    // We don't have the end of the tag, copy up to the start of it and break
                    throw Done{tag_start};
                // We found a tag
                pair tag{tag_start, tag_end+1};
                handleTag(tag, copy_from);
                // Now that we've handled the tag, continue searching from just past the end of it
                if (*tag_end == '>')
                    pos = tag.second; 
                else
                    // If we ended this tag by starting another, don't skip over it
                    pos = tag_end;
            }
        } catch (Done e) {
            // We can push out the nuchanged data now
            assert(noChange);
            noChange(copy_from, e.pos);
            return e.pos;
        }
        return end;
    }

    /** Once we've found a tag, handles checking, then possibly emiting events to change the attribute.
     *
     * If we change one of the attributes, emit the 'noChange' event for the data before the attribute value,
     * then the 'newData' event for the new attribute value, then set the copy_from iterator to the '"' after
     * the attribute value.
     *
     * @param tag The tag that we found
     * @param copy_from The place where one kkk
     * */
    void handleTag(pair tag, iterator& copy_from) {
        // Get the tag name
        pair tag_name = getTagName(tag);
        if (!tag_name) 
            return;
        // See if we care about it
        const std::string& attrib_name = config.getAttrib(tag_name);
        if (attrib_name.empty())
            return;
        // If we care, get the attribute value
        auto attrib = findAttribute(tag, attrib_name);
        if (attrib.first == tag.second)
            return;
        // If we found the attribute we wanted, see if we can get a new value from the config
        std::string newVal = getGoodAttribVal(attrib);
        if (newVal.empty()) 
            return;
        // If there is a new attribute value..
        // Make sure we got given actual event handlers
        assert(noChange);
        assert(newData);
        // Output the unchanged bits
        noChange(copy_from, attrib.first);
        // Send on the new data
        newData(newVal);
        // Get ready for next loop
        copy_from = attrib.second; // Next time the 'unchanged data' will start with the '"' after the attribute value
    }

    /// Takes the start and end of a tag and retuns the tag name.
    pair getTagName(pair tag) {
        auto name_start = tag.first+1;
        auto name_end = std::find_first_of(name_start, tag.second, ws, ws_end);
        // Return the result
        return {name_start, name_end};
    }

    /** Returns the start and end of the attribute value that you want
     * @return the start and end of the iterator value, or {tag.second, tag.second} if the attribute is not found
     */
    pair findAttribute(pair tag, const std::string& attrib_name) {
        struct NotFound{}; /// Exception for if we can't find the attribute
        auto tag_end = tag.second;
        auto find = [=](iterator start, char c) {
            auto res = std::find(start, tag_end, c);
            if (res == tag_end)
                throw NotFound();
            return res;
        };
        auto pos = tag.first;
        while (pos < tag_end) {
            try {
                // Find ' ' .. tagName .. '=' .. '"' .. tagValue .. '"'
                auto space = find(pos, ' ');
                auto equal = find(space+1, '=');
                auto quote1 = find(equal+1, '"');
                auto quote2 = find(quote1+1, '"');
                pos = quote2+1; // Ready for the next run
                // Check if this is the attribute we care about
                if (space+1 < equal) {
                    using std::placeholders::_1;
                    // Skip over multiple adjacent whitespace before the attrib name
                    auto attrib_start = std::find_if_not(space+1, equal, isWS);
                    auto attrib_end = std::find_if(attrib_start, equal, isWS);
                    pair attrib_found{attrib_start, attrib_end};
                    // Case insensitive compare
                    auto lower_compare = [](char a, char b) {
                        return std::tolower(a) == std::tolower(b);
                    };
                    if ((attrib_found.length() == attrib_name.length()) && 
                        (std::equal(attrib_start, attrib_end, attrib_name.begin(), lower_compare)))
                            return {quote1+1, quote2};
                }
            } catch (NotFound) {
                // We couldn't find any more attributes in this tag
                break;
            }
        }
        return {tag_end, tag_end};
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

    /** Takes the start and end of the attribute value that we want and returns a new good value.
     * If we don't have anything to change, just returns the original value
     *
     * @return returns the desired attribute value, or an empty string if no change is to be made
     */
    std::string getGoodAttribVal(pair attrib) {
        std::string attrib_value;
        if (is_relative(attrib)) {
            // Prepend the attrib value with our current location if it's a relative path
            attrib_value.reserve(location.length() + attrib.second - attrib.first);
            std::copy(location.begin(), location.end(), std::back_inserter(attrib_value));
        } else {
            // Just copy the attribute value to the string
            attrib_value.reserve(attrib.second - attrib.first);
        }
        std::copy(attrib.first, attrib.second, std::back_inserter(attrib_value));
        // See if we have a replacement, if we search for /images/abc.gif .. we'll get the CDN for /images/ (if that's in the config)
        // 'found' will be like {"/images/", "http://cdn.supa.ws/images/"}
        auto found = config.findCDNUrl(attrib_value);
        const std::string& base_path=found.first;
        const std::string& cdn_url=found.second;
        // Check if the path we found is a substring of the value
        if (attrib_value.length() > base_path.length()) {
            auto match = std::mismatch(base_path.begin(), base_path.end(), attrib_value.begin());
            if (match.first == base_path.end()) {
                // If the attribute value starts with the base_path, replace the bit after with the new cdn url
                std::string result;
                result.reserve(cdn_url.length() + attrib_value.length() - base_path.length());
                auto out = back_inserter(result);
                std::copy(cdn_url.begin(), cdn_url.end(), out);
                std::copy(match.second, attrib_value.end(), out);
                return result;
            }        
        }
        // Return value unchanged
        return "";
    }

    /** Returns true if 'path' is relative.
     *
     * ie. if it doesn't start with '/', 'http://', or 'https://'
     *
     * @param path We'll check if this is relative, or ablosute
     * @return true of 'path' is relative or empty
     **/
    bool is_relative(const pair& path) {
        if (path)
            return false; // Empty path
        if (path[0] == '/')
            return false; // Absolute path
        // If it can hold at least http://x
        if (path.length() > 7) {
            // Check http and friends
            std::string http{"http"};
            auto match = std::mismatch(http.begin(), http.end(), path.first);
            if (match.first == http.end()) {
                // Check http:// and https://
                auto checkStart = match.first+1;
                if (*checkStart == 's')
                    ++checkStart;
                std::string protocol{"://"};
                auto match = std::mismatch(protocol.begin(), protocol.end(), checkStart);
                if (match.first == protocol.end())
                    return false;
            }
        }
        return true;
    }


};

}
