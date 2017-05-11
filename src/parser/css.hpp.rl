#pragma once

namespace cdnalizer {
namespace parser {

%%{ 
  machine css_impl;
  include css "css.machine.rl";
  css_impl := css;
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
template <typename Iterator>
Iterator parseCSS(Iterator &p, const Iterator& pe,
                  std::function<void(Iterator, Iterator)> path_found) {
  int cs;

  // Data needed for the actions
  auto url_start = p;

  // State machine initialization
  %%write init;

  // State machine code
  %%write exec;

  return p;
}


} /* parser */ 
} /* cdnalizer  */ 
