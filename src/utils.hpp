#pragma once
#include <functional>

namespace cdnalizer {
namespace utils {

/// Our safe std::equal test for use with forward input iterators, with no way of knowing length
template <typename iter1, typename iter2,
          typename predicate>
bool equal(iter1 first1, iter1 last1, iter2 first2, iter2 last2, predicate pred) {
    while ((first1 != last1) && (first2 != last2)) {
        if (!pred(*first1++, *first2++))
            return false;
    }
    return ((first1 == last1) && (first2 == last2));
}

inline bool default_pred(char a, char b) { return a == b; }

template <typename iter1, typename iter2>
bool equal(iter1 first1, iter1 last1, iter2 first2, iter2 last2) {
    return equal(first1, last1, first2, last2, &default_pred);
}

/// @return first position where two ranges differ
template <typename iter1, typename iter2, typename iter3, typename iter4>
std::pair<iter1, iter3> mismatch(iter1 first1, iter2 last1, iter3 first2, iter4 last2) {
    while ( (first1!=last1) && (first2!=last2) && (*first1==*first2) )
        { ++first1; ++first2; }
    return std::make_pair(first1,first2);
}

/// @return first position where two ranges differ
template <typename iter1, typename iter2,
          typename value_type1, typename value_type2,
          typename predicate>
std::pair<iter1, iter2> mismatch(iter1 first1, iter1, iter2 last1, iter2 first2, iter2 last2, predicate pred) {
    while ( (first1!=last1) && (first2!=last2) && (pred(*first1, *first2)) )
        { ++first1; ++first2; }
    return std::make_pair(first1,first2);
}

/// @returns true if the path/url between 'start' and 'end' is relative
template <typename Iterator>
bool is_relative(Iterator start, const Iterator& end) {
    if (start == end)
        return false; // Empty path counts as '/'
    if (*start == '/')
        return false; // Absolute path
    // Check http and friends
    const std::string http = "http";
    typedef std::pair<std::string::const_iterator, Iterator> Match;  
    Match match = utils::mismatch(http.begin(), http.end(), start, end);
    if (match.first == http.end()) {
        // Check http:// and https://
        Iterator checkStart = match.second;
        if (*++checkStart != 's')
            checkStart = match.second;
        const std::string protocol = "://";
        Match match = utils::mismatch(protocol.begin(), protocol.end(), checkStart, end);
        if (match.first == protocol.end())
            return false;
    }
    return true;
}

}
}
