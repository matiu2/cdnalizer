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

// The type of an HTML attribute. For Normal attributes, we treate the whole
// contents as a path. For Style attributes, we invoke the CSS parser.
enum attrib_type { Normal, Style };

// A record of an attribute with its type and location for processesing later
template <typename Iterator>
struct AttribRec {
  attrib_type type = Normal;
  boost::iterator_range<Iterator> location;
};

namespace parser {

using namespace boost::spirit::x3;

template <typename Iterator>
auto getCSSParser(boost::iterator_range<Iterator> &out) {
  auto get = [&out](auto ctx) { out = _attr(ctx); };
  auto double_quoted_path = lit('"') >> raw[+(char_ - (lit('"') | eoi))][get] >> '"';
  auto single_quoted_path = lit('\'') >> raw[+(char_ - (lit('\'') | eoi))][get] >> '\'';
  auto no_quote_path = raw[+(char_ - (lit(')') | eoi))][get];
  auto css_path = double_quoted_path | single_quoted_path | no_quote_path; 
  auto junk_before_path = +(char_ - ("url" | eoi));
  return junk_before_path >> "url" >> '(' >> css_path >> ')';
}

/// @param type What kind of attribute is the upcoming attribute (normal / style)
template <typename Iterator>
auto getHTMLParser(std::vector<AttribRec<Iterator>> &out) {

  // Operations
  attrib_type type(Normal);
  auto is_style = [&type](const auto &) {
    type = Style; };
  auto is_normal = [&type](const auto &) {
    type = Normal; };
  auto get = [&out, &type](const auto &ctx) {
    out.push_back({type, _attr(ctx)});
  };

  // Actual parsers //
  auto tag_start = lexeme[lit('<') >> +alnum >> ' '];
  auto tag_end = lit('>') | lit("/>");
  // Attribute name finders
  auto attrib_name_chars = alnum | '_' | ':';
  auto attrib_name = lexeme[+attrib_name_chars];
  auto attrib_lit_style = lexeme[no_case[lit("style")]][is_style] >> '=';
  auto attrib_normal = attrib_name[is_normal] >> '=';
  auto attrib_name_options = attrib_lit_style | attrib_normal;
  auto bool_attrib = attrib_name;
  // Attribute value finders
  auto no_quote_chars = (char_ - (tag_end | '\'' | '"' | eoi | space));
  auto attrib_no_quotes =
      lexeme[raw[+no_quote_chars][get] >> (tag_end | space)];
  auto attrib_double_quotes =
      lit('"') >> raw[+(char_ - (lit('"') | eoi))][get] >> '"';
  auto attrib_single_quotes =
      lit('\'') >> raw[+(char_ - (lit('\'') | eoi))][get] >> '\'';
  // Bringing all the attribute bits together
  auto attribute =
      (attrib_name_options >>
           (attrib_double_quotes | attrib_single_quotes | attrib_no_quotes)) |
       bool_attrib;
  // Bringing tags and attributes together
  return *(char_ - (tag_start | eoi)) >> tag_start >> +attribute >> tag_end;
}

} /* parser  */

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
  // See if we're looking for css urls or html/xml
  if (isCSS) {
    while (pos != end) {
      boost::iterator_range<iterator> path(pos, end);
      bool ok = parser::phrase_parse(pos, end, parser::getCSSParser(path),
                                     parser::space);
      if (ok) {
        operateOnBuckets(handlePath({path.begin(), path.end()}));
      } else
        break;
    }
  } else {
    while (pos != end) {
      std::vector<AttribRec<iterator>> paths;
      bool ok = parser::phrase_parse(pos, end, parser::getHTMLParser(paths),
                                     parser::space);
      if (!ok)
        break;
      if ((ok) && (!paths.empty())) {
        std::vector<Change> changes;
        changes.reserve(paths.size());
        for (auto &attrib : paths) {
          switch (attrib.type) {
          case Normal: {
            // This is a normal attribute; treat the whole thing as a path
            Change change(handlePath(attrib.location));
            if (!change.empty())
              changes.emplace_back(std::move(change));
            break;
          }
          case Style: {
            // If we have a style attribute, parse through it again, searching
            // for css paths, rather than treat it as a single path in itself.
            auto attribPos = attrib.location.begin();
            const auto attribEnd = attrib.location.end();
            auto css_parser = parser::getCSSParser(attrib.location);
            while (attribPos != attribEnd) {
              bool ok = parser::phrase_parse(attribPos, attribEnd, css_parser,
                                             parser::space);
              if (!ok)
                break;
              Change change = handlePath(attrib.location);
              if (!change.empty()) {
                changes.emplace_back(std::move(change));
              }
            }
            break;
          }
          };
        }
        // Now that all parsing is done for this tag, process the generated changes.
        // We have to do this afterwards because issuing the bucket events
        // can invalidate the iterators, and we can't have that half way through
        // parsing
        switch (changes.size()) {
        case 1:
          operateOnBuckets(changes.front());
        case 0:
          continue;
        default: {
          // We'll make one change for the whole tag
          std::string newTagInnards;
          Change compositeChange{
              {changes.front().path.begin(), changes.back().path.end()},
              0,
              newTagInnards};
          // TODO: experiment if reserving the memory with a scan like this is
          // faster or slower than not reserving the memory, or reserving some
          // guestimate
          newTagInnards.reserve(compositeChange.size());
          auto in = changes.front().path.begin();
          auto out = std::back_inserter(newTagInnards);
          for (Change &change : changes) {
            using namespace std;
            std::string tmp(change.path.begin(), change.path.end());
            cout << "Processing change: path(" << tmp << ") - how much to cut(" << change.howMuchToCut << ") - newData(" << change.newData << ")" << endl;
            // Copy up to the beginning of the next change (first time it will
            // do nothing)
            while (in != change.path.begin())
              *out++ = *in++;
            // Output the new data
            std::copy(change.newData.begin(), change.newData.end(), out);
            // Skip over the start of the path that we've replaced
            std::advance(in, change.howMuchToCut);
          }
          compositeChange.howMuchToCut =
              std::distance(compositeChange.path.begin(),
                            compositeChange.path.end()) -
              (std::distance(changes.back().path.begin(),
                             changes.back().path.end()) -
               changes.back().howMuchToCut -
               ((!compositeChange.newData.empty()) &&
                        (compositeChange.newData.back() == '/') &&
                        (*compositeChange.path.end() == '/')
                    ? 0
                    : 1));
          using namespace std;
          std::string tmp(compositeChange.path.begin(), compositeChange.path.end());
          cout << "Composite change: path(" << tmp << ") - how much to cut("
               << compositeChange.howMuchToCut << ") - newData("
               << compositeChange.newData << ")" << endl;
          operateOnBuckets(compositeChange);
        }
        };
      }
    }
  }
  // We can push out the unchanged data now
  assert(noChange);
  return noChange(nextNoChangeStart, end);
}
}
