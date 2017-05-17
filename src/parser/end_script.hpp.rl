#pragma once

namespace cdnalizer {
namespace parser {

%%{ 
  machine end_script;
  include end_script "end_script.machine.rl";
}%%

// State machine exports
%%write exports;

// State machine data
%%write data;

/// Finds </script> .. returns true if found
/// p is a reference because when dealing with Apache bucket brigades, it can change, and we changed it also
/// pe is a const reference because apache bucket brigade splitting may change it (but we don't change it).
template <typename Iterator>
bool findEndScriptTag(Iterator &p, const Iterator &pe) {
  int cs;

  // State machine initialization
  %%write init;

  // State machine code
  %%write exec;

  return p >= end_script_first_final;
}


} /* parser */ 
} /* cdnalizer  */ 
