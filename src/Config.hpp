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
#include <iostream>

#include "pair.hpp"

namespace std {
    /// So we can do searching and sorting on our Container below
    inline bool operator <(const string& a, const pair<string, string>& b) { return a < b.first; }
    inline bool operator <(const pair<string, string>& a, const string& b) { return a.first < b; }
}

using Container = std::map<std::string, std::string>;

const Container default_tag_attrib {
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
        {"link", "href"},
        {"layer", "src"},
        {"object", "usemap"},
        {"q", "url"},
        {"script", "url"},
        {"style", "src"}
};

namespace cdnalizer {

class Config {
public:
    using CDNPair = std::pair<const std::string&, const std::string&>;
    /// Thrown when we can't find a suitable CDN url for a base path
    struct NotFound {};
private:
    /// Map of paths to urls, eg. {{"/images/", "http://cdn.supa.ws/images/"}}
    Container path_url;
    /// Pairs of 'tag to change' + 'attribute to change'. eg. {{"img","src"}, {"a", "href"}}
    Container tag_attrib;
    const std::string empty={};
    /// Lookup in a string dict, using a 'pair'
    /// @returns the value, or an empty string if not found
    template <typename iterator>
    const std::string& lookup(const Container& container, const pair<iterator>& tag) const {
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
        if (result == container.cbegin())
            throw NotFound();
        --result;
        return *result;
    }
public:
    /** Initialize the configuration.
     *
     * @param path_url a map of paths we'll find in the html, and their corresponding cdn urls. eg {["/images", "http://cdn.supa.ws/imgs"}}
     * @param tag_attrib A map of tag names to the attribute we should check. Must all be lower case. eg. {{"a", "href"}}
     */
    Config(Container&& path_url={}, Container&& tag_attrib={}) : path_url(path_url), tag_attrib(tag_attrib.empty() ? default_tag_attrib : tag_attrib ) {}
    /** Copy constructor */
    Config(const Config&) = default;
    /// @return the attribute that we care about for a tag name, or an empty string if not found
    template <typename iterator>
    const std::string& getAttrib(const pair<iterator>& tag) const { return lookup(tag_attrib, tag); }
    /// Finds the apprpriate path base. If you're searching for /images/abc.gif, and we have '/images' in the config you'll get that.
    /// @return the key that was matched, and the CDN url for that we should be serving
    CDNPair findCDNUrl(const std::string& tag) const { return search(path_url, tag); }
    /// Add a path-url pair, for later lookup
    void addPath(std::string path, std::string url) {
        path_url.insert(std::make_pair(path, url));
    }
    /// Include the values from another config object
    Config& operator +=(const Config& other) {
        for (auto pair : other.path_url) {
            auto inserted = path_url.insert(pair);
            // If it was already there, update the value
            if (!inserted.second)
                inserted.first->second = pair.second;
        }
        return *this;
    }
};

}
