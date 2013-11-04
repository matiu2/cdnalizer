#pragma once
/** Configuration Holder
 *
 * # Config attributes
 *  * Tag Attrib:
 *    * img src
 *    * a href
 *  * Values to change:
 *    * /my/images/ ==> https://cdn.supa.ws/my_images/
 *    * something   ==> http://cdn.supa.ws/something/somewhere/prefix_
 */

#include <vector>
#include <string>
#include <map>

#include "pair.hpp"

namespace cdnalizer {

class Config {
public:
    using Container = std::map<std::string, std::string>;
    using CDNPair = std::pair<const std::string&, const std::string&>;
private:
    /// Map of paths to urls, eg. {{"/images/", "http://cdn.supa.ws/images/"}}
    Container path_url;
    /// Pairs of 'tag to change' + 'attribute to change'. eg. {{"img","src"}, {"a", "href"}}
    Container tag_attrib = {
        {"a", "href"},
        {"applet", "codebase"},
        {"area", "href"},
        {"audio", "src"},
        {"base", "href"},
        {"bgsound", "src"},
        {"blockquote", "cite"},
        {"embed", "src"},
        {"frame", "src"},
        {"iframe", "src"},
        {"img", "src"},
        {"input", "src"},
        {"layer", "src"},
        {"object", "usemap"},
        {"q", "url"},
        {"script", "url"},
        {"style", "src"}
    };
    const std::string empty={};
    /// Lookup in a string dict, using a 'pair'
    /// @returns the value, or an empty string if not found
    const std::string& lookup(const Container& container, const pair& tag) const {
        auto result = lower_bound(container.cbegin(), container.cend(), tag);
        if (result == container.cend())
            return empty;
        return result->first == tag ? result->second : empty;
    }
    /// finds the best candidate for a match
    /// @recturns the value
    CDNPair search(const Container& container, const std::string& tag) const {
        // upper_bound always returns one after the one we want,
        // wether the key is an exact match or not
        auto result = upper_bound(container.cbegin(), container.cend(), tag);
        return *--result;
    }
public:
    /** Initialize the configuration.
     *
     * @param path_url a map of paths we'll find in the html, and their corresponding cdn urls. eg {["/images", "http://cdn.supa.ws/imgs"}}
     */
    Config(Container&& path_url) : path_url(path_url) {}
    /** Initialize the configuration.
     *
     * @param path_url a map of paths we'll find in the html, and their corresponding cdn urls. eg {["/images", "http://cdn.supa.ws/imgs"}}
     * @param tag_attrib A map of tag names to the attribute we should check. Must all be lower case. eg. {{"a", "href"}}
     */
    Config(Container&& path_url, Container&& tag_attrib) : path_url(path_url), tag_attrib(tag_attrib) {}
    /// @return the attribute that we care about for a tag name, or an empty string if not found
    const std::string& getAttrib(const pair& tag) const { return lookup(tag_attrib, tag); }
    /// Finds a close match. If you're searching for /images/abc.gif, and we have '/images' you'll get that.
    /// @return the key that was matched, and the CDN url for that we should be serving
    CDNPair findCDNUrl(const std::string& tag) const { return search(path_url, tag); }
};

}
