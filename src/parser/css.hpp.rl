#pragma once

#include <string>
#include <functional>
#include <boost/range/iterator_range.hpp>

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
/// p is a reference because when dealing with Apache bucket brigades, it can change, and we changed it also
/// pe is a const reference because apache bucket brigade splitting may change it (but we don't change it).
/// cs is a reference to the current state of parsing
template <typename Iterator>
boost::iterator_range<Iterator> parseCSS(Iterator &p, Iterator pe, int &cs) {

  Iterator path_start;

  int path_start_state = css_start;

  // If current state is undefined, set it to our start state
  if (cs == -1)
    cs = css_start;

  // State machine initialization
  %%write init;

  // State machine code
  %%write exec;

  // If we get here we've found the path_start but not the path end return just
  // the path_start, and an empty iterator
  if (path_start != decltype(path_start)())
    return {path_start, Iterator{}};
}


} /* parser */ 
} /* cdnalizer  */ 
