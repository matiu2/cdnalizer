#include "Parser.hpp"

#include "pathUtils.hpp"

#include "parser/css.hpp"
// TODO #include "parser/js.hpp"
// TODO #include "parser/json.hpp"
// TODO #include "parser/html.hpp"

namespace cdnalizer {

Parser::Result Parser::parseCSSBlock(Iterator p, Iterator pe) {

  Iterator path_start = nullptr;
  std::string newPath =
      parser::parseCSS(p, pe, cs, path_start, &fixPath<Iterator>);

  /// CSS Parsing found a new path to output
  if (newPath != "")
    return {path_start, newPath};

  /// We didn't find a new path, but we found the start of one
  if (path_start != nullptr) {
    // Record the start of it
    halfPath.erase();
    std::copy(path_start, pe, std::back_inserter(halfPath));
    // Tell the caller to split this bucket at this path start, in case we need
    // to make modifications to it later
    return {path_start, {}};
  }
}

Parser::Result Parser::parseJSBlock(Iterator p, Iterator pe) {
}

Parser::Result Parser::parseJSONBlock(Iterator p, Iterator pe) {
}

Parser::Result Parser::parseHTMLBlock(Iterator p, Iterator pe) {
}


} /* cdnalizer  */ 
