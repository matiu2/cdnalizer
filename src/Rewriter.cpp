#include "Rewriter.hpp"

#include <algorithm>
#include <iterator>   // find search
#include <cctype>     // tolower

namespace cdnalizer {

struct Rewriter {
    using iterator = std::string::iterator;
    using const_iterator = std::string::const_iterator;
    using iterators = std::pair<iterator, iterator>;
    using const_iterators = std::pair<const_iterator, const_iterator>;
    struct Done{}; /// Exception to say we're done parsing

    const std::string& location;
    const Config& config;
    const std::string& in;
    const_iterator real_end = in.end();
    std::string out;

    Rewriter(const std::string& location, const Config& config, const std::string& html)
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
                        std::string newVal = getGoodVal(attrib);
                        copy_until(attrib.first);                         // Copy up to the start of the attrib value
                        std::copy(newVal.begin(), newVal.end(), out_pos); // Copy the new attribute Value
                        in_pos += newVal.size();
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

    /// Searches through the input stream and returns a const_iterator pair to the start and end of a tag
    const_iterators findNextTag(const_iterator in_pos) {
        auto start = std::find(in_pos, real_end, '<');
        if (start == real_end) 
            throw Done();
        auto end = std::find(start, real_end, '>');
        if (end == real_end) 
            throw Done();
        in_pos = end+1;
        return {start, end};
    }

    /// Takes the start and end of a tag and retuns the tag name. Returns "" for closing tags.
    std::string getTagName(const_iterators tag) {
        auto start = tag.first+1;
        auto end = std::find(start, tag.second, ' ');
        // Ignore closing tags
        if ((end-start >= 1) && (*start == '/'))
            return "";
        // Return the result
        std::string result;
        result.reserve(end-start);
        std::copy(start, end, back_inserter(result));
        return result;
    }

    /** Returns the start and end of the attribute value that you want
     * @return the start and end of the iterator value, or {tag.second, tag.second} if the attribute is not found
     */
    const_iterators findAttribute(const_iterators tag, const std::string& attrib_name) {
        struct NotFound{}; /// Exception for if we can't find the attribute
        auto end = tag.second;
        auto find = [=](const_iterator start, const_iterator::value_type c) {
            auto res = std::find(start, end, c);
            if (res == end)
                throw NotFound();
            return res;
        };
        try {
            auto start = tag.first;
            while (start < end) {
                // Find ' ' .. tagName .. '=' .. '"' .. tagValue .. '"'
                auto space = find(start, ' ');
                auto equal = find(space+1, '=');
                auto quote1 = find(equal+1, '"');
                auto quote2 = find(quote1+1, '"');
                start = quote2+1; // Ready for the next run
                // Check if this is the attribute we care about
                if (space+1 < equal) {
                    using Char = const_iterator::value_type;
                    // (Use search to ignore extra white space before or after attrib name)
                    auto attrib_found = std::search(space+1, equal, attrib_name.begin(), attrib_name.end(), [](Char a, Char b) {
                        return std::tolower(a) == std::tolower(b);
                    });
                    if (attrib_found != equal)
                        return {quote1+1, quote2};
                }
            }
        } catch (NotFound) {
            // Just needed to break out of the loop from inside the lambda
        }
        return {end, end};
    }

    /** Takes the start and end of the attribute value that we want and returns a new good value.
     * If we don't have anything to change, just returns the original value
     *
     * @return returns the desired attribute value.
     */
    std::string getGoodVal(const_iterators attrib) {
        // Get the old value
        std::string value;
        value.reserve(attrib.second - attrib.first);
        std::copy(attrib.first, attrib.second, back_inserter(value));
        // See if we have a replacement
        auto found = config.paths.find(value);
        return found != config.paths.end() ? found->second : value;
    }

};

std::string rewriteHTML(const std::string& location,
                        const Config& config,
                        const std::string& html) {
    Rewriter r{location, config, html};
    return r.out;
}


}
