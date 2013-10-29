#pragma once
/// A class that behaves a bit like a string, but work off of long lived const char* data, and help avoid copying of data
#include <algorithm>
#include <cassert>

namespace cdnalizer {

using iterator = const char*;
using base_pair = std::pair<iterator, iterator>;

struct pair : base_pair {
    pair() : base_pair() {};
    pair(iterator first, iterator second) : base_pair(first, second) {};
    int length() const { return second-first; }
    char operator[](int i) const {
        assert(i < length());
        return first[i];
    }
    operator bool() const { return length() > 0; }
    bool operator <(const pair& other) const {
        return std::lexicographical_compare(
            first, second,
            other.first, other.second);
    }
    bool operator <(const std::string& other) const {
        return std::lexicographical_compare(
            first, second,
            other.begin(), other.end());
    }
    bool operator ==(const pair& other) const {
        if ((first == other.first) && (second == other.second))
            return std::equal(first, second, other.first);
        return false;
    }
    bool operator ==(const std::string& other) const {
        if (length() == other.length())
            return std::equal(first, second, other.cbegin());
        return false;
    }
};

bool operator <(const std::string& a, const pair& b) {
        return std::lexicographical_compare(
            a.begin(), a.end(),
            b.first, b.second);
}

bool operator ==(const std::string& a, const pair& b) { return b == a; }


}
