#pragma once

#include <functional>

#include "apache/contentTypes.hpp"
#include "apache/BucketEvent.hpp"
#include "path.hpp"

#include "parser/css.hpp"

/// Parses everything we need and keeps state

namespace cdnalizer {

template <typename Iterator, typename Block>
struct Parser {
public:
private:
  enum Found {b_only, e_only, be, nothing};
  /// Current state
  int cs = -1;
  apache::ContentType ct;
  const std::string& location;
  static const Iterator empty;
  /// Path start info
  Iterator pathStartIt;
  Block pathStartBlock;

public:
  /// Main constructor
  Parser(apache::ContentType contentType, const std::string &location)
      : ct(contentType), location(location) {}
  /// Parses the next block of data
  /// We return a point in the block where we need for the
  /// block; false if it's in the middle of parsing a path.
  void parseNextBlock(Block block, Iterator &p, Iterator pe, std::string &newData) {
    while (p != pe) {
      boost::iterator_range<Iterator> path;
      switch (ct) {
      case apache::HTML:
        break;
      case apache::CSS:
        path = parser::parseCSS(p, pe, cs);
        break;
      case apache::JSON:
        break;
      case apache::JS:
        break;
      };

      /// Handle the path that we found
      Found found;
      if (path.begin() != empty)
        if (path.end() != empty)
          found = be;
        else
          found = b_only;
      else if (path.end() != empty)
        found = e_only;
      else
        found = nothing;
      switch (found) {
      case b_only: {
        pathStartIt = p;
        pathStartBlock = block;
      }
      case e_only: {
        // We just found the end of a path here, we must have found the start in
        // a previous block
        assert(pathStartIt != empty);
        handlePath(pathStartIt, pathStartBlock, p, block);
      }
      case be: {
        handlePath(p, pe);
      }
      case nothing: {
        // We found nothing, we can just handle the next block
        // We should be at the end
        assert(p == pe);
      }
      };
    }
  }
};

} /* cdnalizer  */ 
