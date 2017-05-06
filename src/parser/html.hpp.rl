#pragma once

#include <boost/range/iterator_range.hpp>

namespace cdnalizer {
namespace parser {

%%{ 
  machine tag;
  include tag "html.machine.rl";
}%%

// State machine exports
%%write exports;

// State machine data
%%write data;

/// Parses some HTML, looking for url() functions
/// @param A reference to the pointer to the start of the data. This will be incremented as our search continues
/// @param pe A const reference to the pointer to the end of the input
/// @param attrib_found This function will be called every time an HTML attribute is found,
///        The first two iterators are the start and end of the attribute name
///        The second two iterators are the start and end of the attribute value
/// @param tag_found -- returns pointers to the beginning and end of an html tag name
/// 
/// p is a reference because when dealing with Apache bucket brigades, it can change, and we changed it also
/// pe is a const reference because apache bucket brigade splitting may change it (but we don't change it).
template <typename Iterator>
bool parseHTMLTag(
    Iterator &p, const Iterator &pe,
    std::function<void(boost::iterator_range<Iterator>)> tag_found,
    std::function<void(boost::iterator_range<Iterator>,
                       boost::iterator_range<Iterator>)>
        attrib_found) {
  int cs;

  // Data needed for the actions
  auto tag_name_start = p;
  auto attrib_name_start = p;
  auto attrib_name_end = p;
  auto attrib_val_start = p;

  // State machine initialization
  %%write init;

  // State machine code
  %%write exec;

  return cs >= tag_first_final;
}


} /* parser */ 
} /* cdnalizer  */ 

