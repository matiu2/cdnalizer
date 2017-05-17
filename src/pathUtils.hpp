#pragma once
/// Utilities for handling paths found

#include "parser/path.hpp"

namespace cdnalizer {


/// Fixes a path.
/// @param start the beginning of the path to fix
/// @param end the end of the path to fix
/// @param location the current location in the directory tree according to Apache
/// @param the current hostname according to Apache
/// @returns an empty string if nothing needs changing
template <typename Iterator>
std::string fixPath(Iterator start, Iterator end, const std::string &location,
                    const std::string &hostname) {
  /// If the path is not to a static resource (is to a .php) just forget about it
  if (!parser::isPathStatic(path))
    return "";
}

} /* cdnalizer */ 
