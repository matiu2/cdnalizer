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
#include "utils.hpp"

namespace std {
    /// So we can do searching and sorting on our Container below
    inline bool operator <(const string& a, const pair<string, string>& b) { return a < b.first; }
    inline bool operator <(const pair<string, string>& a, const string& b) { return a.first < b; }
}

namespace cdnalizer {

typedef std::map<std::string, std::string> Container;

const Container& get_default_tag_attrib();

class Config {
public:
    /// A Pair type that *holds* 2 strings - used for search parameters
    typedef std::pair<std::string, std::string> CDNPair;
    /// holds references to 2 strings - used for search results
    typedef std::pair<const std::string&, const std::string&> CDNRefPair;
    /// Thrown when we can't find a suitable CDN url for a base path
    struct NotFound {};
private:
    /// The base/prefix that goes in front of relative urls
    std::string base_location;
    /// Map of paths to urls, eg. {{"/images/", "http://cdn.supa.ws/images/"}}
    Container path_url;
    /// Pairs of 'tag to change' + 'attribute to change'. eg. {{"img","src"}, {"a", "href"}}
    Container tag_attrib;
    static const std::string empty;
    /// Lookup in a string dict, using a 'pair'
    /// @tparam iterator pchar or string iterator
    /// @returns the value, or an empty string if not found
    template <typename iterator>
    const std::string& lookup(const Container& container, const pair<iterator>& tag) const {
        Container::const_iterator result = lower_bound(container.begin(), container.end(), tag);
        if (result == container.end())
            return empty;
        return result->first == tag ? result->second : empty;
    }
    /// finds the best candidate for a match
    /// @recturns the value
    CDNRefPair search(const Container& container, const std::string& tag) const {
        // upper_bound always returns one after the one we want,
        // wether the key is an exact match or not
        Container::const_iterator result = upper_bound(container.begin(), container.end(), tag);
        if (result == container.begin())
            throw NotFound();
        --result;
        return *result;
    }
    /// Absolutelize a path/url in place
    void absolutelize(std::string& path) {
        std::string::const_iterator begin = path.begin();
        std::string::const_iterator end = path.end();
        if (utils::is_relative(begin, end))
            path.insert(0, base_location);
    }
    /// Ensure base_location ends in '/'
    void ensureSlashOnEnd() {
        if (base_location.empty() || (*base_location.rbegin() != '/'))
            base_location.append("/");
    }
public:
    /** Initialize the configuration.
     *
     * @param path_url a map of paths we'll find in the html, and their corresponding cdn urls. eg {["/images", "http://cdn.supa.ws/imgs"}}
     * @param base_location a base location to add on to all relative key urls; defaults to "/'
     * @param tag_attrib A map of tag names to the attribute we should check. Must all be lower case. eg. {{"a", "href"}}
     */
    Config(Container path_url=Container(), const char* base_location="/", Container tag_attrib=Container()) 
        : base_location(base_location),  path_url(path_url), tag_attrib(tag_attrib.empty() ? get_default_tag_attrib() : tag_attrib ) 
    { ensureSlashOnEnd(); }
    /** Initialize the configuration.
     *
     * @param path_url a map of paths we'll find in the html, and their corresponding cdn urls. eg {["/images", "http://cdn.supa.ws/imgs"}}
     * @param tag_attrib A map of tag names to the attribute we should check. Must all be lower case. eg. {{"a", "href"}}
     * @param a base location to add on to all relative key urls; defaults to "/'
     */
    Config(const Container& path_url, const Container& tag_attrib, const char* base_location="/") 
        : base_location(base_location), path_url(path_url), tag_attrib(tag_attrib.empty() ? get_default_tag_attrib() : tag_attrib )
    { ensureSlashOnEnd(); }
    /** Copy constructor */
    Config(const Config& other) : base_location(other.base_location), path_url(other.path_url), tag_attrib(other.tag_attrib) {}
    /// @return the attribute that we care about for a tag name, or an empty string if not found
    template <typename iterator>
    const std::string& getAttrib(const pair<iterator>& tag) const { return lookup(tag_attrib, tag); }
    /// Finds the apprpriate path base. If you're searching for /images/abc.gif, and we have '/images' in the config you'll get that.
    /// @return the key that was matched, and the CDN url for that we should be serving
    CDNRefPair findCDNUrl(const std::string& tag) const { return search(path_url, tag); }
    /// Add a path-url pair, for later lookup
    void addPath(std::string path, std::string url) {
        absolutelize(path);
        absolutelize(url); // NOTE: Someone may change ./css/ to ./resources/css .. might not be http://some.cdn/css
        path_url.insert(std::make_pair(path, url));
    }
    /// Include the values from another config object
    Config& operator +=(const Config& other) {
        for (Container::const_iterator iPair=other.path_url.begin(); iPair != other.path_url.end(); iPair++) {
            const Container::value_type& pair = *iPair;
            std::pair<Container::iterator, bool> inserted = path_url.insert(pair);
            // If it was already there, update the value
            if (!inserted.second)
                inserted.first->second = pair.second;
        }
        return *this;
    }
};

}
