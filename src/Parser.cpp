#include "Parser.hpp"

#include "pathUtils.hpp"

#include "parser/css.hpp"
// TODO #include "parser/js.hpp"
// TODO #include "parser/json.hpp"
// TODO #include "parser/html.hpp"

namespace cdnalizer {

void Parser::parseCSSBlock(Iterator &p, Iterator pe, std::string &newPath) {
  parser::parseCSS(p, pe, cs, newPath, &fixPath<Iterator>);
}

void Parser::parseJSBlock(Iterator &p, Iterator pe) {
}

void Parser::parseJSONBlock(Iterator &p, Iterator pe) {
}

void Parser::parseHTMLBlock(Iterator &p, Iterator pe) {
}


} /* cdnalizer  */ 
