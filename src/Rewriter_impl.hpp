#pragma once
/**
 * © Copyright 2014 Matthew Sherborne. All Rights Reserved.
 * © Copyright 2017 Matthew Sherborne. All Rights Reserved.
 * License: Apache License, Version 2.0 (See LICENSE.txt)
 **/
#include "Rewriter.hpp"

#include "Config.hpp"
#include "utils.hpp"
#include "parser/css.hpp"
#include "parser/path.hpp"
#include "parser/html.hpp"

#include <algorithm>
#include <cctype>   // tolower
#include <iterator> // find search
#include <cassert>

#include <boost/hana/if.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/type.hpp>

namespace cdnalizer {

using namespace std::string_literals; // enables s-suffix for std::string literals  

template <typename iterator>
iterator rewriteHTML(const std::string &server_url, const std::string &location,
                     const Config &config, iterator start, iterator end,
                     RangeEvent<iterator> noChange, DataEvent newData,
                     bool isCSS) {

  iterator nextNoChangeStart = start;

  const std::string empty;

  struct Change {
    boost::iterator_range<iterator> path;
    size_t howMuchToCut;
    const std::string &newData;
    bool empty() const {
      return (path.empty()) && (howMuchToCut == 0) && (newData.empty());
    }
    size_t size() const {
      return std::distance(path.begin(), path.end());
    }
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
  auto handlePath = [&](boost::iterator_range<iterator> path_range) -> Change {
    // Work out the path value we'll search the config DB for
    // range.begin() is the character after the first "quote"
    // range.end() is the last quote
    //
    // Input variables we care about:
    //
    //  * path_range: The path we're replacing
    //  * location: The path that the HTML/CSS that generated this path comes
    //  from
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
      if (location.back() != '/') {
        canonical.push_back('/');
        --skipOverCount;
      }
      std::copy(path_range.begin(), path_range.end(),
                std::back_inserter(canonical));
      // The written url will be 'images/x', but canonical will be
      // '/blog/images/x'
      // later we'll find that '/blog/images/' is in the config, so skip over 12
      // chars but we only want to skip over 6 chars ('images/')
      // so we take away len('/blog') which is our location + 2
      // for the slash
      skipOverCount -= location.size();
      return canonical;
    };

    auto canonicalFromAbsolutePath = [&location, &path_range, &server_url,
                                      &skipOverCount]() {
      if (!server_url.empty()) {
        // TODO: In reality there may be several server_url aliases; we should
        // probably check all of them here
        auto match = utils::mismatch(server_url.cbegin(), server_url.cend(),
                                     path_range.begin(), path_range.end());
        if (match.first == server_url.cend()) {
          // Also we want to skip over the extra length of that url in the
          // incoming data
          skipOverCount += server_url.length();
          return std::string(match.second, path_range.end());
        }
      }
      // No changes needed; copy the whole path as is
      return std::string(path_range.begin(), path_range.end());
    };

    std::string canonical(
        utils::is_relative(path_range.begin(), path_range.end())
            ? canonicalFromRelativePath()
            : canonicalFromAbsolutePath());

    // Now we know what to search for in our map
    // eg. canonical='/images/fun.gif'

    // See if we have a replacement, if we search for /images/abc.gif .. we'll
    // get the CDN for /images/ (if that's in the config)
    // 'found' will be like {"/images/", "http://cdn.supa.ws/images/"}
    auto found(config.findCDNUrl(canonical));
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

    size_t howMuchToCut = std::distance(path_range.begin(), path_range.end()) -
                          (canonical.size() - base_path.size());

    return {path_range, howMuchToCut, cdn_url};
  };

  // Emit the change handlers
  auto operateOnBuckets = [&](Change change) {
    // Make sure we got given actual event handlers
    assert(noChange);
    assert(newData);
    assert(!change.empty());

    // Find the distance from the end of the pas to pos, so we can reset pos later
    auto distance = change.size();

    // Output everything before the start of this path  as unchanged
    nextNoChangeStart = noChange(nextNoChangeStart, change.path.begin());

    // At this point **all iterators** apart from nextNoChangeStart may be
    // **corrupt and useless** and should not be used.
    // This is because the noChange callback may split the bucket and
    // alter the bucket brigade

    // Send the new data (which is the new cdn url)
    newData(change.newData);

    // We need to return the new position
    auto result = nextNoChangeStart;
    std::advance(result, distance);

    // Skip over the parts of the path that we replaced.
    std::advance(nextNoChangeStart, change.howMuchToCut);

    // Avoid "//" in output
    if ((!change.newData.empty()) && (change.newData.back() == '/') &&
        (*nextNoChangeStart == '/'))
      ++nextNoChangeStart;

    // Return the new end of path, so parsing can continue
    return result; };

  // Find a path to replace in the html/css
  iterator pos = start;

  // See if we're looking for css urls or html/xml
  if (isCSS) {
    while (pos != end) {
      parser::parseCSS(pos, end, std::function<void(iterator, iterator)>([&](
                                     iterator path_begin, iterator path_end) {
                         auto path =
                             boost::make_iterator_range(path_begin, path_end);
                         if (!parser::isPathStatic(path))
                           return;
                         Change change = handlePath(path);
                         if (!change.empty()) {
                           pos = operateOnBuckets(std::move(change));
                         }
                       }));
    }
  } else {
    std::function<void(boost::iterator_range<iterator>,
                       boost::iterator_range<iterator>)>
        onAttributeFound = [&pos, &handlePath, &operateOnBuckets](
            boost::iterator_range<iterator> name,
            boost::iterator_range<iterator> value) {
          if (name != "style"s) {
            // This is a normal attribute; treat the whole thing as a path
            if (parser::isPathStatic(value)) {
              Change change(handlePath(value));
              if (!change.empty())
                pos = operateOnBuckets(std::move(
                    change)); // Set the new pos, because we are mid-parse
            }
          } else {
            // If we have a style attribute, parse through it again, searching
            // for css paths, rather than treat it as a single path in itself.
            auto attribPos = value.begin();
            auto attribEnd = value.end();
            assert(pos == attribEnd); // Assume pos is the same as attribEnd
            while (attribPos != attribEnd) {
              parser::parseCSS(
                  attribPos, attribEnd,
                  std::function<void(iterator, iterator)>([&](
                      iterator path_begin, iterator path_end) {
                    auto path =
                        boost::make_iterator_range(path_begin, path_end);
                    if (!parser::isPathStatic(path))
                      return;
                    Change change = handlePath(path);
                    if (!change.empty()) {
                      // After operating on buckets, it will return
                      // change.path.end() and all other iterators will be
                      // invalid, so we need to grab distances now
                      auto dist_to_attr_pos =
                          std::distance(change.path.end(), attribPos);
                      auto dist_to_attr_end =
                          std::distance(change.path.end(), attribEnd);
                      auto end_of_path = operateOnBuckets(std::move(change));
                      // All the other iterators are now invalid, because a
                      // bucket
                      // has been split
                      attribPos = attribEnd = pos = end_of_path;
                      std::advance(attribPos, dist_to_attr_pos);
                      std::advance(attribEnd, dist_to_attr_end);
                      pos = attribEnd;
                    }
                  }));
            }
          }
        };
    while (pos != end)
      if (!parser::parseHTML(pos, end, onAttributeFound))
        break;
  };
  // We can push out the unchanged data now
  assert(noChange);
  return noChange(nextNoChangeStart, end);
}
}
