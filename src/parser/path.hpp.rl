#pragma once

#include <boost/range/iterator_range.hpp>

namespace cdnalizer {
namespace parser {

%%{ 
  machine path_impl;
  include path "path.machine.rl";
  path_impl := path;
}%%

// State machine exports
%%write exports;

// State machine data
%%write data;

/// Parses a path
/// @param A reference to the pointer to the start of the data. This will be incremented as our search continues
/// @param pe A const reference to the pointer to the end of the input
/// Returns 'true' if the path is static.
/// p is a reference because when dealing with Apache bucket brigades, it can change, and we changed it also
/// pe is a const reference because apache bucket brigade splitting may change it (but we don't change it).
template <typename Iterator>
bool isPathStatic(boost::iterator_range<Iterator> path) {
  auto p = path.begin();
  const auto &pe = path.end();
  const auto &eof = pe;
  int cs;

  // State machine initialization
  %%write init;

  // State machine code
  %%write exec;

  return true;
}


} /* parser */ 
} /* cdnalizer  */ 

