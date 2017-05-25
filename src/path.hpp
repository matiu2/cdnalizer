#pragma once

#include "utils.hpp"

#include <boost/range/iterator_range.hpp>

namespace cdnalizer {

/// Takes a relative path and returns a canonical URL in a string.
/// Furthermore it updates `skipOverCount`, which is later used
/// to skip over original text that won't be output in the final text
///
/// @param skipOverCount will be filled with how many letters to skip over
/// @param location Our path location according to Apache
/// @param path The path as presented by the original text
/// @tparam Path path can be any sequence of chars, either a
///         boost::iterator_range or a std::string will be fine
template <typename Path>
std::string canonicalFromRelativePath(int &skipOverCount,
                                      const std::string &location,
                                      const Path &path) {
  // The canonical path will start with location
  std::string canonical(location);
  // If it doesn't end in a '/' add one and remember that the slash is not in
  // the original path
  if (location.back() != '/') {
    canonical.push_back('/');
    --skipOverCount;
  }
  std::copy(path.begin(), path.end(),
            std::back_inserter(canonical));
  // The written url will be 'images/x', but canonical will be
  // '/blog/images/x'
  // later we'll find that '/blog/images/' is in the config, so skip over 12
  // chars (len("/blog/image")) but we only want to skip over 6 chars ('images/')
  // so we take away len('/blog') which is our location + 2 for the slash
  skipOverCount -= location.size();
  return canonical;
}

/// Takes an absolute path and returns a canonical URL in a string.
/// It updates `skipOverCount`, which is later used to skip over original text
/// that won't be output in the final text
///
/// @param skipOverCount will be incremented and/or decremented with how many
///        letters to skip over
/// @param location Our path location according to Apache
/// @param path The path as presented by the original text
/// @param server_url The protocol and port of the server, eg. https://supa.ws
/// @tparam Path path can be any sequence of chars, either a
///         boost::iterator_range or a std::string will be fine
template <typename Path>
std::string
canonicalFromAbsolutePath(int &skipOverCount, const std::string &location,
                          const Path &path, const std::string &server_url) {
  if (!server_url.empty()) {
    // TODO: In reality there may be several server_url aliases; we should
    // probably check all of them here
    auto match = utils::mismatch(server_url.cbegin(), server_url.cend(),
                                 path.begin(), path.end());
    if (match.first == server_url.cend()) {
      // If the path in the source text contains the server_url,
      // we want to skip over that extra length in the source
      skipOverCount += server_url.length();
      // We'll return the part after the hostname
      return std::string(match.second, path.end());
    }
  }
  // The original path didn't contain the server_url so No changes needed;
  // return the whole path as is
  return std::string(path.begin(), path.end());
}

using FindCDNURL = std::function<const std::pair<std::string, std::string> &(
                    const std::string &)>;

template <typename Path>
std::pair<int, const std::string &>
handlePath(int &skipOverCount, const std::string &location, const Path &path,
           const std::string &server_url, FindCDNURL findCDNUrl) {
  // First get the canonical version of the string
  std::string canonical(
      utils::is_relative(start, end)
          ? canonicalFromRelativePath(skipOverCount, location, path)
          : canonicalFromAbsolutePath(skipOverCount, location, path,
                                      server_url));

  // Now we know what to search for in our map
  // eg. canonical='/images/fun.gif'

  // See if we have a replacement, if we search for /images/abc.gif .. we'll
  // get the CDN for /images/ (if that's in the config)
  // 'found' will be {'path we care about', 'what it should look like'}
  // eg. {"/images/", "http://cdn.supa.ws/images/"}
  auto found(findCDNUrl(canonical));
  if (found.first.empty() && found.second.empty()) {
    // We found nothing
    return {{}, 0, empty};
  }

  skipOverCount += found.first.size();

  const std::string &base_path = found.first;
  const std::string &cdn_url = found.second;

  // We have three possible situations here:
  // 1. * path_range = "/images/x.jpg"
  //    * base_path = "/images/"
  //    * canonical = "/images/"
  //    * cdn_url = "http://cdn.supa.ws/images/"
  // 2. * path_range = "http://www.supa.ws/images/x.jpg"
  //    * base_path = "/images/"
  //    * canonical = "/images/x.jpg"
  //    * cdn_url = "http://cdn.supa.ws/images/"
  // 3. * path_range = "../images/x.jpg"
  //    * base_path = "/images/"
  //    * canonical = "/images/x.jpg"
  //    * cdn_url = "http://cdn.supa.ws/images/"

  size_t howMuchToCut = std::distance(path.begin(), path.end()) -
                        (canonical.size() - base_path.size());

  return {howMuchToCut, cdn_url};
}

} /* cdnalizer */
