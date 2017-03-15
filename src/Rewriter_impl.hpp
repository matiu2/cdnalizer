#pragma once
/**
 * © Copyright 2014 Matthew Sherborne. All Rights Reserved.
 * License: Apache License, Version 2.0 (See LICENSE.txt)
 **/
#include "Rewriter.hpp"

#include "Config.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cctype>   // tolower
#include <iterator> // find search

namespace cdnalizer {

template <typename iterator, typename char_type>
iterator rewriteHTML(const std::string &server_url, const std::string &location,
                     const Config &config, iterator start, iterator end,
                     RangeEvent<iterator> noChange, DataEvent newData) {
  using pair = cdnalizer::pair<iterator>;

  /// An exception we throw when we have finished parsing the input, and catch
  /// in the main loop.
  struct Done {
    iterator pos; /// The character that the next run should start with. (One
                  /// past the last character we read).
  };

  iterator nextNoChangeStart = start;

  /** Returns true if 'path' is relative.
   *
   * ie. if it doesn't start with '/', 'http://', or 'https://'
   *
   * @param path We'll check if this is relative, or ablosute
   * @return true of 'path' is relative or empty
   **/
  auto is_relative = [](const pair &path) {
    return utils::is_relative(path.first, path.second);
  };

  /** Takes the start and end of the path value generated and emits
   * events, possibly changing the value.
   *
   * If we don't have anything to change, just outputs nothing and leaves
   * nextNoChangeStart unchanged
   *
   * @param path_range The range in the input, of the path value we care about
   * @returns true if all iterators after path start need recalculating
   *
   */
  auto handlePath = [&](const pair &path_range) {
    // Work out the path value we'll search the config DB for
    // range.begin() is the character after the first "quote"
    // range.end() is the last quote
    //
    // Input variables we care about:
    //
    //  * path_range: The path we're replacing
    //  * location: The path that the HTML/CSS that generated this path comes from
    //  * server_url: The protocol and hostname part of the request
    //  * config.findCDNUrl(canonical): the map of base_urls to be replaced by
    //    cdn URLs
    //
    // Intermediate steps:
    //
    //  * We create a canonical version of the url for the key when looking up
    //    the cdn url
    //
    // What we change:
    //
    //  * We split the bucket before the URL, sending everything before it down
    //    the pipe unchanged. This invalidates the path_range.end() iterator, so
    //    we do it near the end.
    //  * We transmit a new bucket, just containing the new base part of the
    //    path
    //  * We then split the bucket again, droping the path part we replaced, and
    //    sending everything after it out unchanged.
    //
    // Example scenarios:
    //
    //  1. base_path is "/images/fun.gif"
    //     location is irrelevant because base_path is absolute
    //     server_url is irrelevant too
    //     In our CDNUrl map we have "/images/" = "https:://cdn.supa.ws/images/"
    //     We'll transmit the new bucket: "https:://cdn.supa.ws/images/"
    //     We'll drop the data "/images/"
    //     We'll start the next bucket at "fun.gif...."
    //
    //  2. base_path is "fun.gif"
    //     location is "/images"
    //     New bucket: "https:://cdn.supa.ws/images/"
    //     Drop: nothing
    //     Start again at: "fun.gif..."
    //
    //  3. base_path is "http://supa.ws/images/fun.gif"
    //     server_url is "http://supa.ws"
    //     New bucket:  "https:://cdn.supa.ws/images/"
    //     Skip over: "http://supa.ws/images/"
    //
    //

    // After transmitting, we'll need to know how much of the url to skip over.
    // eg. we don't need to transmit our server URL
    int skipOverCount = 0;

    /// Gets our canonical path
    auto canonicalFromRelativePath = [&skipOverCount, &location,
                                      &path_range]() {
      std::string canonical(location);
      if (location.back() != '/')
        canonical.push_back('/');
      std::copy(path_range.first, path_range.second,
                std::back_inserter(canonical));
      // The written url will be 'images/x', but canonical will be
      // '/blog/images/x'
      // later we'll find that '/blog/images/' is in the config, so skip over 12
      // chars but we only want to skip over 6 chars ('images/')
      // so we take away len('/blog') which is our location + 1
      // for the slash
      skipOverCount -= location.size();
      --skipOverCount;
      return canonical;
    };

    auto canonicalFromAbsolutePath = [&location, &path_range, &server_url,
                                      &skipOverCount]() {
      if (!server_url.empty()) {
        // TODO: In reality there may be several server_url aliases; we should
        // probably check all of them here
        auto match = utils::mismatch(server_url.cbegin(), server_url.cend(),
                                     path_range.first, path_range.second);
        if (match.first == server_url.cend()) {
          // Also we want to skip over the extra length of that url in the
          // incoming data
          skipOverCount += server_url.length();
          return std::string(match.second, path_range.second);
        }
      }
      // No changes needed; copy the whole path as is
      return std::string(path_range.first, path_range.second);
    };

    std::string canonical(is_relative(path_range)
                              ? canonicalFromRelativePath()
                              : canonicalFromAbsolutePath());

    // Now we know what to search for in our map
    // eg. canonical='/images/fun.gif'

    // See if we have a replacement, if we search for /images/abc.gif .. we'll
    // get the CDN for /images/ (if that's in the config)
    // 'found' will be like {"/images/", "http://cdn.supa.ws/images/"}
    auto found(config.findCDNUrl(canonical));
    if (found.first.empty() && found.second.empty()) {
      return path_range.second;
    }

    skipOverCount += found.first.size();

    const std::string &base_path = found.first;
    const std::string &cdn_url = found.second;

    // If canonical starts with base_path replace it with cdn_url
    if ((canonical.length() > base_path.length()) &&
        (std::equal(base_path.cbegin(), base_path.cend(), canonical.begin()))) {

      // Make sure we got given actual event handlers
      assert(noChange);
      assert(newData);

      // Output everything before the start of this path  as unchanged
      nextNoChangeStart = noChange(nextNoChangeStart, path_range.first);

      // At this point **all iterators** apart from nextNoChangeStart may be
      // **corrupt and useless** and should not be used.
      // This is because the noChange callback may split the bucket and
      // alter the bucket brigade

      // Send the new data (which is the new cdn url)
      newData(cdn_url);

      // Skip over the parts of the path that we replaced.
      std::advance(nextNoChangeStart, skipOverCount);

      // Avoid "//" in output
      if ((!cdn_url.empty()) && (cdn_url.back() == '/') && (*nextNoChangeStart == '/'))
        ++nextNoChangeStart;
      return nextNoChangeStart;
    } else {
      return path_range.second;
    }
  };

  auto find_quote = [&](iterator start, auto predicate) {
    // Find a quotes start and end
    auto result = std::find_if(start, end, predicate);
    // If there are no more tags, send all the data
    if (result == end)
      throw Done{end};
    return result;
  };

  // Find a path to replace in the html/css
  iterator pos = start;
  try {
    while (pos != end) {
      auto quote_start =
          find_quote(pos, [](char c) { return (c == '\'') || (c == '"'); });
      auto pred = [d = *quote_start](auto c) { return (c == d); };
      auto quote_end = find_quote(++quote_start, pred);
      handlePath(pair{quote_start, quote_end});
      pos = quote_end;
      ++pos;
    }
  } catch (Done e) {
    // We can push out the unchanged data now
    assert(noChange);
    return noChange(nextNoChangeStart, e.pos);
  }
  return end;
}
}
