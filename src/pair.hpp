#pragma once
/// A class that behaves a bit like a string, but work off of long lived const char* data, and help avoid copying of data

#include "utils.hpp"

#include <algorithm>
#include <cassert>

namespace cdnalizer {

/** A pair of iterators into a const char array.
 *
 * Designed to minimize data copying when working with const char arrays.
 *
 * It gives you some of the features you know and love from string,
 * but is very minimal as it only stores two pointers, to the start and one past the end of the array.
 *
 */
template<typename iterator>
struct pair : std::pair<iterator, iterator> {
#ifdef HAVE_CPP11
    using ParentClass = std::pair<iterator, iterator>;
#else
    typedef std::pair<iterator, iterator> ParentClass;
#endif
    pair() : ParentClass() {};
    pair(const pair& other) : ParentClass(other.first, other.second) {}
    pair(iterator first, iterator second) : ParentClass(first, second) {};
    /// Subscript into the selection
    char operator[](size_t i) const {
        iterator output = this->first;
        while ((i-- > 0) && (output++ != this->second));
        return *output;
    }
    /// @returns true if we have some data
    operator bool() const { return this->first != this->second; }
    /// Allows sorting with other paths
    bool operator <(const pair& other) const {
        return std::lexicographical_compare(
            this->first, this->second,
            other.first, other.second);
    }
    /// Allows sorting amongst normal strings
    bool operator <(const std::string& other) const {
        return std::lexicographical_compare(
            this->first, this->second,
            other.begin(), other.end());
    }
    /// Compare to another path - @returns true if they both contain the same leterns
    bool operator ==(const pair& other) const {
        return utils::equal(this->first, this->second, other.first, other.second);
    }
    /// Compare to a string
    bool operator ==(const std::string& other) const {
        iterator begin = this->first;
        iterator end = this->second;
        return utils::equal(begin, end, other.begin(), other.end());
    }
};

/// Allows sorting amongst a list of strings
template<typename iterator>
inline bool operator <(const std::string& a, const pair<iterator>& b) {
        return std::lexicographical_compare(
            a.begin(), a.end(),
            b.first, b.second);
}

/// Allows sorting and searching amongst a map of string-string pairs
template<typename iterator>
inline bool operator <(const std::pair<std::string, std::string>& a, const pair<iterator>& b) {
#ifdef HAVE_CPP11
        return std::lexicographical_compare( a.first.cbegin(), a.first.cend(), b.first, b.second);
#else
        return std::lexicographical_compare( a.first.begin(), a.first.end(), b.first, b.second);
#endif
}

/// Allows comparing to a string, when the string is the lhs argument
template<typename iterator>
inline bool operator ==(const std::string& a, const pair<iterator>& b) { return b == a; }


}
