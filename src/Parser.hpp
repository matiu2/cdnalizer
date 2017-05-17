#pragma once

#include <functional>

#include "apache/contentTypes.hpp"
#include "apache/BucketEvent.hpp"

/// Parses everything we need and keeps state

namespace cdnalizer {

struct Parser {
public:
  using Iterator = char*;
  using Result = apache::BucketEvent;

private:
  /// Current state
  int cs = -1;
  apache::ContentType ct;
  const std::string& location;
  /// If the parser hits a path that crosses a boundary, it'll store what it
  /// already has here
  std::string halfPath;
  Result parseCSSBlock(Iterator p, Iterator pe);
  Result parseJSBlock(Iterator p, Iterator pe);
  Result parseJSONBlock(Iterator p, Iterator pe);
  Result parseHTMLBlock(Iterator p, Iterator pe);
  /// Called internally when a path is found. It uses AnyIterator, because it
  /// can be called with std::string::iterator as well as char*
  template <typename AnyIterator>
  Result onPathFound(Iterator p, AnyIterator path_start, AnyIterator path_end);

public:
  /// Main constructor
  Parser(apache::ContentType contentType, const std::string &location)
      : ct(contentType), location(location) {}
  /// Parses the next block of data
  /// We return a point in the block where we need for the 
  /// block; false if it's in the middle of parsing a path.
  Result parseNextBlock(Iterator p, Iterator pe) {
    switch (ct) {
      case HTML:
        return parseHTMLBlock(p, pe);
      case CSS:
        return parseCSSBlock(p, pe);
      case JSON:
        return parseJSONBlock(p, pe);
      case JS:
        return parseJSBlock(p, pe);
    };
  }
};
  
} /* cdnalizer  */ 
