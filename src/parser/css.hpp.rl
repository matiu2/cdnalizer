#pragma once

#include <string>
#include <functional>

namespace cdnalizer {
namespace parser {

%%{ 
  machine css;
  include css "css.machine.rl";
}%%

// State machine exports
%%write exports;

// State machine data
%%write data;

/// Parses some CSS, looking for url() functions
/// @param A reference to the pointer to the start of the data. This will be incremented as our search continues
/// @param pe A const reference to the pointer to the end of the input
/// @param path_found This function will be called every time a path is found,
///        passing two iterators, the first letter of the path, and one past the end.
/// p is a reference because when dealing with Apache bucket brigades, it can change, and we changed it also
/// pe is a const reference because apache bucket brigade splitting may change it (but we don't change it).
/// cs is a reference to the current state of parsing
template <typename Iterator>
std::string
parseCSS(Iterator &p, const Iterator &pe, int &cs, Iterator &path_start,
         std::function<std::string(Iterator, Iterator)> path_found) {

  // If current state is undefined, set it to our start state
  if (cs == -1)
    cs = css_start;

  // State machine initialization
  %%write init;

  // State machine code
  %%write exec;

  return "";
}


} /* parser */ 
} /* cdnalizer  */ 
