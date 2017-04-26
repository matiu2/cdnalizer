#pragma once
/**
 * © Copyright 2014 Matthew Sherborne. All Rights Reserved.
 * © Copyright 2017 Matthew Sherborne. All Rights Reserved.
 * License: Apache License, Version 2.0 (See LICENSE.txt)
 **/
#include "Rewriter.hpp"

#include "Config.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cctype>   // tolower
#include <iterator> // find search

#include <boost/spirit/home/x3.hpp>
#include <boost/hana/if.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/type.hpp>

namespace cdnalizer {

// The type of an HTML attribute. For Normal attributes, we treate the whole
// contents as a path. For Style attributes, we invoke the CSS parser.
enum attrib_type { Normal, Style };

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
  return raw[junk_before_path >> "url" >> '(' >> css_path >> ')'];
}

/// @param onAttributeFound A function that will be called for each attribute
template <typename Iterator>
auto getHTMLParser(
    std::function<void(attrib_type, boost::iterator_range<Iterator>)>
        onAttributeFound) {

  // Operations
  attrib_type type(Normal);
  auto is_style = [&type](const auto &) {
    type = Style; };
  auto is_normal = [&type](const auto &) {
    type = Normal; };
  auto get = [onAttributeFound, &type](const auto &ctx) {
    onAttributeFound(type, _attr(ctx));
  };

  // Actual parsers //
  auto tag_start = lexeme[lit('<') >> +alnum >> ' '];
  auto xml_thing = lit("<!") >> +(char_ - (lit(">") | eoi)) >> lit(">");
  auto comment = lit("<!--") >> +(char_ - (lit("-->") | eoi)) >> lit("-->");
  auto end_tag = lexeme[lit("</") >> +alnum >> '>'];
  auto tag_end = lit('>') | lit("/>");
  // Attribute name finders
  auto attrib_name_chars = alnum | '_' | ':' | '-';
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
  auto a_tag = tag_start >> +attribute >> tag_end;
  auto no_attributes = lexeme[lit('<') >> +alnum >> '>'];
  auto not_a_tag = (end_tag | comment | xml_thing | no_attributes);
  return raw[*(char_ - (lit('<') | eoi)) >> (not_a_tag | a_tag | eoi)];
}

/// Returns a parser for finding if a path is server only (.php / .pl)
/// This parser is faster than 'getPathParser', but the iterator must support
/// reverse iteration
auto getFastPathParser() {
  // This parser exepcts a reverse iterator (from the end of the path back to
  // the beginning) because if we find that the line ends with '.php' straight
  // away, we give a fast result.
  // We also want to return true for stuff like: /some/url
  auto php = lit("php."); // Reverse of '.php'
  auto pl = lit("lp.");   // Reverse of '.pl'
  auto py = lit("yp.");   // Reverse of '.py'
  auto extensions = php | pl | py;
  return raw[extensions | +(char_ - (lit('?') | '/')) >> '?' >> extensions];
}

/// Returns a parser for finding if a path is server only (.php / .pl)
auto getPathParser() {
  auto php = lit(".php"); // Reverse of '.php'
  auto pl = lit(".pl");   // Reverse of '.pl'
  auto py = lit(".py");   // Reverse of '.py'
  auto extensions = php | pl | py;
  return raw[+(char_ - (lit('.') | eoi | '?')) >> extensions >> ('?' | eoi)];
}

} /* parser  */

/// A filter to remove .php or .pl files from being CDNalized
/// Returns true if the path should be served from the CDN
template <typename Iter>
bool checkPath(boost::iterator_range<Iter> path) {
  auto supportsReverse = boost::hana::is_valid(
      [](auto &&it) -> decltype(*--it) {});
  auto isExecutable = boost::hana::if_(
      supportsReverse(path.begin()),
      [](auto path) {
        auto begin = std::make_reverse_iterator(path.end());
        auto end = std::make_reverse_iterator(path.begin());
        return parser::phrase_parse<decltype(begin)>(
            begin, end, parser::getFastPathParser(), parser::space);
      },
      [](auto path) {
        return parser::phrase_parse(path.begin(), path.end(),
                                    parser::getPathParser(), parser::space);
      });
  return !isExecutable(path);
}

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
    return result;
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
        if (checkPath(path))
          operateOnBuckets(handlePath({path.begin(), path.end()}));
      } else
        break;
    }
  } else {
    std::function<void(attrib_type, boost::iterator_range<iterator>)>
        onAttributeFound = [&pos, &handlePath, &operateOnBuckets](
            attrib_type type, boost::iterator_range<iterator> value) {
          switch (type) {
          case Normal: {
            // This is a normal attribute; treat the whole thing as a path
            if (checkPath(value)) {
              Change change(handlePath(value));
              if (!change.empty())
                pos = operateOnBuckets(std::move(
                    change)); // Set the new pos, because we are mid-parse
            }
            break;
          }
          case Style: {
            // If we have a style attribute, parse through it again, searching
            // for css paths, rather than treat it as a single path in itself.
            auto attribPos = value.begin();
            auto attribEnd = value.end();
            decltype(value) path;
            auto css_parser = parser::getCSSParser(path);
            assert(pos == attribEnd); // Assume pos is the same as attribEnd

            while (attribPos != attribEnd) {
              bool ok = parser::phrase_parse(attribPos, attribEnd, css_parser,
                                             parser::space);
              if (!ok)
                break;

              if (!checkPath(path))
                continue;

              Change change = handlePath(path);
              if (!change.empty()) {
                // After operating on buckets, it will return
                // change.path.end() and all other iterators will be invalid,
                // so we need to grab distances now
                auto dist_to_attr_pos =
                    std::distance(change.path.end(), attribPos);
                auto dist_to_attr_end =
                    std::distance(change.path.end(), attribEnd);
                auto end_of_path = operateOnBuckets(std::move(change));
                // All the other iterators are now invalid, because a bucket
                // has been split
                attribPos = attribEnd = pos = end_of_path;
                std::advance(attribPos, dist_to_attr_pos);
                std::advance(attribEnd, dist_to_attr_end);
                pos = attribEnd;
              }
            }
            break;
          }
          };
        };
    while (pos != end) {
      bool ok = parser::phrase_parse(
          pos, end, parser::getHTMLParser(onAttributeFound), parser::space);
      if (!ok)
        break;
    }
  }
  // We can push out the unchanged data now
  assert(noChange);
  return noChange(nextNoChangeStart, end);
}
}
