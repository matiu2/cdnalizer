#include "Rewriter.hpp"

#include "Config.hpp"

#include <algorithm>
#include <iterator>   // find search
#include <cctype>     // tolower

namespace cdnalizer {

struct RewriterSettings {
};

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
        DataEvent newData)
        :
        location{location}, config{config}, start{start}, end{end},
        noChange{noChange}, newData{newData}
        {}

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
                auto tag_end = std::find(tag_start, end, '>');
                if (tag_end == end)
                    // We don't have the end of the tag, copy up to the start of it and break
                    throw Done{tag_start};
                pair tag{tag_start, tag_end+1};
                // Get the tag name
                pair tag_name = getTagName(tag);
                if (tag_name) {
                    // See if we care about it
                    const std::string& attrib_name = config.getAttrib(tag_name);
                    if (!attrib_name.empty()) {
                        // If we care, get the attribute value
                        auto attrib = findAttribute(tag, attrib_name);
                        if (attrib.first != tag.second) {
                            // If we found the attribute we wanted, replace it's value
                            // Output the unchanged bits
                            assert(noChange);
                            noChange(copy_from, attrib.first);
                            // Output the new value
                            std::string newVal = getGoodAttribVal(attrib);
                            assert(newData);
                            newData(newVal);
                            // Get ready for next loop
                            copy_from = attrib.second; // The '"' quote after the attribute value
                        }
                    }
                }
                // We found a tag, but the name and or attrib didn't match
                pos = tag.second;
            }
        } catch (Done e) {
            // We can push out the nuchanged data now
            assert(noChange);
            noChange(copy_from, e.pos);
            return e.pos;
        }
        return end;
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
        auto find = [=](iterator start, iterator::value_type c) {
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
                    auto isWS = std::bind(&Impl::isWS, this, _1);
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

    /** Takes the start and end of the attribute value that we want and returns a new good value.
     * If we don't have anything to change, just returns the original value
     *
     * @return returns the desired attribute value.
     */
    std::string getGoodAttribVal(const_iterators attrib) {
        // Get the old value
        std::string value;
        value.reserve(attrib.second - attrib.first);
        std::copy(attrib.first, attrib.second, back_inserter(value));
        insertDefaultPath(value); // Prepend it with our default location if possible
        // See if we have a replacement
        auto found = config.paths.upper_bound(value);
        if (found-- != config.paths.begin()) {
            // Check if the path we found is a substring of the value
            auto match = std::mismatch(found->first.begin(), found->first.end(), value.begin());
            if (match.first == found->first.end()) {
                // If the value starts with the path we found, replace that path part with the new url
                std::string result;
                result.reserve(found->second.size() + value.size() - found->first.size());
                auto out = back_inserter(result);
                std::copy(found->second.begin(), found->second.end(), out);
                std::copy(match.second, value.end(), out);
                return result;
            }        
        }
        return value;
    }

};

void rewriterHTML(const std::string& location, const Config& config, const char* data,
                  RangeEvent noChange, DataEvent newData, EndEvent finished) {
    while (in_pos < real_end) {
        // Check if we care about it
        std::string tagName = getTagName(tag);
        if (!tagName.empty()) {
            // If it's not a closing or empty tag, see if we care about it
            auto found = config.tag_attrib.find(tagName);                
            if (found != config.tag_attrib.end()) {
                // If we care, get the attribute value
                auto attrib = findAttribute(tag, found->second);
                if (attrib.first < attrib.second) {
                    // If we found the attribute we wanted, replace it's value
                    std::string newVal = getGoodVal(attrib);
                    copy_until(attrib.first);                         // Copy up to the start of the attrib value
                    std::copy(newVal.begin(), newVal.end(), out_pos); // Copy the new attribute Value
                    in_pos = attrib.second;
                }
            }
        }
        copy_until(tag.second); // Finish copying the tag, ready for the next run
    }
}

class Rewriter {
public:
    using iterator = char*;
    using const_iterator = const char*;
    using iterators = std::pair<iterator, iterator>;
    using const_iterators = std::pair<const_iterator, const_iterator>;
private:
    const std::string& location;
    const Config& config;
public:
};

struct Rewriter {
    struct Done{}; /// Exception to say we're done parsing

    const std::string& location;
    const Config& config;
    const std::string& in;
    const_iterator real_end = in.end();
    std::string out;

    Rewriter(const std::string& location,
             const Config& config,
             const char* data,
             RangeEvent noChange,
             DataEvent newData,
             EndEvent finished);
    Impl(const std::string& location, const Config& config, const std::string& html)
        : location(location), config(config), in(html)
    { 
        // Rewrite the space for the output straight off the bat, plus 1K for probable expansions
        out.reserve(in.size() + 1024);
        const_iterator in_pos{in.begin()};
        auto out_pos = std::back_inserter(out);
        auto copy_until = [&in_pos, &out_pos](const_iterator until) {
            std::copy(in_pos, until, out_pos);
            in_pos = until;
        };
        try {
            while (in_pos < real_end) {
                // Find the next tag
                auto tag = findNextTag(in_pos);
                // Check if we care about it
                std::string tagName = getTagName(tag);
                if (!tagName.empty()) {
                    // If it's not a closing or empty tag, see if we care about it
                    auto found = config.tag_attrib.find(tagName);                
                    if (found != config.tag_attrib.end()) {
                        // If we care, get the attribute value
                        auto attrib = findAttribute(tag, found->second);
                        if (attrib.first < attrib.second) {
                            // If we found the attribute we wanted, replace it's value
                            std::string newVal = getGoodVal(attrib);
                            copy_until(attrib.first);                         // Copy up to the start of the attrib value
                            std::copy(newVal.begin(), newVal.end(), out_pos); // Copy the new attribute Value
                            in_pos = attrib.second;
                        }
                    }
                }
                copy_until(tag.second); // Finish copying the tag, ready for the next run
            }
        }  catch (Done) {
            // Just needed to break out of the loop
        }
        // Copy whatever's left
        copy_until(in.end());
    }


    bool isWS(const_iterator::value_type c) {
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

    /** Takes an attribute value and prepends our default location in the front if it's a relative path.
     *  Changes the value in place.
     *  @param path - will be changed in place, prepending the current location if it's a relative path
     */
    void insertDefaultPath(std::string& path) {
        if (path.empty())
            return; // Empty path
        if (path[0] == '/')
            return; // Absolute path
        // Check http
        std::string http{"http://"};
        auto match = std::mismatch(http.begin(), http.end(), path.begin());
        if (match.first == http.end())
            return; // http:// is an absolute path
        // Check https
        std::string https{"https://"};
        match = std::mismatch(https.begin(), https.end(), path.begin());
        if (match.first == https.end())
            return; // https:// is an absolute path
        // This is a relative path, prepend our current location
        path = location + path;
    }

};

}
