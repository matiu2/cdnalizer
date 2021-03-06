#pragma once
/** Configuration Holder
 *
 * © Copyright 2014 Matthew Sherborne. All Rights Reserved.
 * License: Apache License, Version 2.0 (See LICENSE.txt)
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

using Container = std::map<std::string, std::string>;


namespace cdnalizer {

class Config {
public:
  /// A Pair type that *holds* 2 strings - used for search parameters
  using CDNPair = std::pair<std::string, std::string>;
  /// holds references to 2 strings - used for search results
  using CDNRefPair = std::pair<const std::string &, const std::string &>;

private:
  /// The base/prefix that goes in front of relative urls
  std::string base_location;
  /// Map of paths to urls, eg. {{"/images/", "http://cdn.supa.ws/images/"}}
  Container path_url;
  static const std::string empty;
  /// finds the best candidate for a match
  /// @returns the value or two empty strings if nothing is found
  CDNRefPair search(const Container &container, const std::string &path) const {
    // upper_bound always returns one after the one we want,
    // wether the key is an exact match or not
    auto result = upper_bound(container.cbegin(), container.cend(), path);
    if (result == container.cbegin())
      return {{}, {}};
    --result;
    return *result;
  }
  /// Absolutelize a path/url in place
  void absolutelize(std::string &path) {
    if (utils::is_relative(path.cbegin(), path.cend()))
      path.insert(0, base_location);
  }
  /// Ensure base_location ends in '/'
  void ensureSlashOnEnd() {
    if (base_location.empty() || (base_location.back() != '/'))
      base_location.append("/");
  }

public:
  /** Initialize the configuration.
   *
   * @param path_url a map of paths we'll find in the html, and their
   * corresponding cdn urls. eg {["/images", "http://cdn.supa.ws/imgs"}}
   * @param base_location a base location to add on to all relative key urls;
   * defaults to "/'
   */
  Config(Container &&path_url = {}, const char *base_location = "/")
      : base_location{base_location}, path_url(path_url) {
    ensureSlashOnEnd();
  }
  /** Copy constructor */
  Config(const Config &) = default;
  /// Finds the apprpriate path base. If you're searching for /images/abc.gif,
  /// and we have '/images' in the config you'll get that.
  /// @return the key that was matched, and the CDN url for that we should be
  ///         serving. If nothing is found, return two empty strings
  CDNRefPair findCDNUrl(const std::string &path) const {
    return search(path_url, path);
  }
  /// Add a path-url pair, for later lookup
  void addPath(std::string path, std::string url) {
    absolutelize(path);
    absolutelize(url); // NOTE: Someone may change ./css/ to ./resources/css ..
                       // might not be http://some.cdn/css
    path_url.insert(std::make_pair(path, url));
  }
  /// Include the values from another config object
  Config &operator+=(const Config &other) {
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
