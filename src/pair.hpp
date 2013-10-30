#pragma once
/// A class that behaves a bit like a string, but work off of long lived const char* data, and help avoid copying of data
#include <algorithm>
#include <cassert>

namespace cdnalizer {

using iterator = const char*;
using base_pair = std::pair<iterator, iterator>;

/** A pair of iterators into a const char array.
 *
 * Designed to minimize data copying when working with const char arrays.
 *
 * It gives you some of the features you know and love from string,
 * but is very minimal as it only stores two pointers, to the start and one past the end of the array.
 *
 */
struct pair : base_pair {
    pair() : base_pair() {};
    pair(iterator first, iterator second) : base_pair(first, second) {};
    /// @return the length of the selection
    size_t length() const { 
        assert(second>first); // You have a pair (string) with a negative length
        return second-first;
    }
    /// Subscript into the selection
    char operator[](int i) const {
        assert(i < length());
        return first[i];
    }
    /// @returns true if we have some data
    operator bool() const { return length() > 0; }
    /// Allows sorting with other paths
    bool operator <(const pair& other) const {
        return std::lexicographical_compare(
            first, second,
            other.first, other.second);
    }
    /// Allows sorting amongst normal strings
    bool operator <(const std::string& other) const {
        return std::lexicographical_compare(
            first, second,
            other.begin(), other.end());
    }
    /// Compare to another path - @returns true if they both contain the same leterns
    bool operator ==(const pair& other) const {
        if (length() == other.length())
            return std::equal(first, second, other.first);
        return false;
    }
    /// Compare to a string
    bool operator ==(const std::string& other) const {
        if (length() == other.length())
            return std::equal(first, second, other.cbegin());
        return false;
    }
};

/// Allows sorting amongst a list of strings
bool operator <(const std::string& a, const pair& b) {
        return std::lexicographical_compare(
            a.begin(), a.end(),
            b.first, b.second);
}

/// Allows comparing to a string, when the string is the lhs argument
bool operator ==(const std::string& a, const pair& b) { return b == a; }


}
