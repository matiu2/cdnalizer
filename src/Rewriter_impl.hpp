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

#include <boost/spirit/home/x3.hpp>

namespace cdnalizer {

namespace parser {

using namespace boost::spirit::x3;

//auto css_path = lit('(') >> +(char_ - (lit(')') | eoi)) >> ')';
auto css_path = +char_;

//auto tag_start = lexeme[lit('<') >> alpha];
//auto attrib_no_quotes = lexeme[lit('=') >> raw[+(char_ - (space | '>' | eoi))]];
//auto attrib_double_quotes = lit('=') >> lit('"') >> raw[+(char_ - (lit('"') | eoi))] >> '"';
//auto attrib_single_quotes = lit('=') >> lit('\'') >> raw[+(char_ - (lit('\'') | eoi))] >> '\'';
//auto inner_tag_junk = *(char_ - (eoi | '=' | '>'));
//auto attribute = (attrib_double_quotes | attrib_no_quotes | attrib_single_quotes);
//auto tag = tag_start >> +(inner_tag_junk >> attribute >> inner_tag_junk) >> '>';
auto tag = *char_;

} /* parser  */

template <typename iterator>
iterator rewriteHTML(const std::string &server_url, const std::string &location,
                     const Config &config, iterator start, iterator end,
                     RangeEvent<iterator> noChange, DataEvent newData,
                     bool isCSS) {
  /// An exception we throw when we have finished parsing the input, and catch
  /// in the main loop.
  struct Done {
    iterator pos; /// The character that the next run should start with. (One
                  /// past the last character we read).
  };

  iterator nextNoChangeStart = start;

  const std::string empty;

  struct Change {
    boost::iterator_range<iterator> path;
    size_t howMuchToCut;
    const std::string &newData;
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
      std::copy(path_range.begin(), path_range.end(),
                std::back_inserter(canonical));
      // The written url will be 'images/x', but canonical will be
      // '/blog/images/x'
      // later we'll find that '/blog/images/' is in the config, so skip over 12
      // chars but we only want to skip over 6 chars ('images/')
      // so we take away len('/blog') which is our location + 1
      // for the slash
      skipOverCount -= location.size();
      if (location.back() != '/')
        --skipOverCount;
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

  auto operateOnBuckets = [&](Change change) {
    // Make sure we got given actual event handlers
    assert(noChange);
    assert(newData);

    if (change.path.empty())
      return;

    // Output everything before the start of this path  as unchanged
    nextNoChangeStart = noChange(nextNoChangeStart, change.path.begin());

    // At this point **all iterators** apart from nextNoChangeStart may be
    // **corrupt and useless** and should not be used.
    // This is because the noChange callback may split the bucket and
    // alter the bucket brigade

    // Send the new data (which is the new cdn url)
    newData(change.newData);

    // Skip over the parts of the path that we replaced.
    std::advance(nextNoChangeStart, change.howMuchToCut);

    // Avoid "//" in output
    if ((!change.newData.empty()) && (change.newData.back() == '/') &&
        (*nextNoChangeStart == '/'))
      ++nextNoChangeStart;
  };

  // Find a path to replace in the html/css
  iterator pos = start;
  try {
    // See if we're looking for css urls or html/xml
    if (isCSS) {
      while (pos != end) {
        boost::iterator_range<iterator> path(pos, end);
        std::string tmp;
        bool ok = parser::phrase_parse(
            pos, end, parser::css_path, parser::space, tmp);
        if (ok) {
          operateOnBuckets(handlePath({path.begin(), path.end()}));
        }
      }
    } else {
      while (pos != end) {
        std::vector<boost::iterator_range<iterator>> paths;
        std::string tmp;
        bool ok = parser::phrase_parse(
            pos, end, parser::tag,
            parser::space, tmp);
        if ((ok) && (!paths.empty())) {
          std::vector<Change> changes;
          paths.push_back({pos, end});
          changes.reserve(paths.size());
          std::transform(paths.begin(), paths.end(), std::back_inserter(changes), handlePath);
          if (changes.size() != 0) {
            std::string newTagInnards;
            Change finalChange{
                {changes.front().path.begin(), changes.back().path.end()},
                0,
                newTagInnards};
            iterator copyFrom = end;
            auto out = std::back_inserter(newTagInnards);
            for (Change& change : changes) {
              if (change.path.empty())
                continue;
              if (copyFrom != end)
                std::copy(copyFrom, change.path.begin(), out);
              iterator start = change.path.begin();
              std::advance(start, change.howMuchToCut);
              std::copy(start, change.path.end(), out);
              copyFrom = change.path.end();
            }
            operateOnBuckets(std::move(finalChange));
          }
        }
      }
    }

  } catch (Done e) {
    // We can push out the unchanged data now
    assert(noChange);
    return noChange(nextNoChangeStart, e.pos);
  }
  return end;
}
}
