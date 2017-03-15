#pragma once
/**
 * © Copyright 2014 Matthew Sherborne. All Rights Reserved.
 * License: Apache License, Version 2.0 (See LICENSE.txt)
 **/
#include <functional>

namespace cdnalizer {
namespace utils {

/// Our safe std::equal test for use with forward input iterators, with no way of knowing length
template <typename iter1, typename iter2,
          typename value_type1=typename iter1::value_type, typename value_type2=typename iter2::value_type,
          typename predicate=std::function<bool(value_type1, value_type2)>>
bool equal(iter1 first1, iter1 last1, iter2 first2, iter2 last2, predicate pred) {
    while ((first1 != last1) && (first2 != last2)) {
        if (!pred(*first1++, *first2++))
            return false;
    }
    return ((first1 == last1) && (first2 == last2));
}

template <typename T1, typename T2>
bool default_pred(T1 a, T2 b) { return a == b; }

template <typename iter1, typename iter2,
          typename value_type1 = typename iter1::value_type,
          typename value_type2 = typename iter2::value_type>
bool equal(iter1 first1, iter1 last1, iter2 first2, iter2 last2) {
  return utils::equal(first1, last1, first2, last2,
                      &default_pred<value_type1, value_type2>);
}

/// @return a pair of iterators:
///         (1st input string location that doesn't match 2nd,
///          2nd input string location that doesn't match 1st)
/// If nothing matches, it'll return (first1, first2),
/// If everything matches, it'll return (last1, last2),
/// If 1st matches the start of 2nd, it'll return
///             (last1, first2 + distance(first1, last1)),
/// If 2nd matches the start of 1nd, it'll return
///       (first1 + distance(first2, last2, last2),
template <typename iter1, typename iter2>
std::pair<iter1, iter2> mismatch(iter1 first1, iter1 last1, iter2 first2,
                                 iter2 last2) {
  while ((first1 != last1) && (first2 != last2) && (*first1 == *first2)) {
    ++first1;
    ++first2;
  }
  return std::make_pair(first1, first2);
}

/// @return first position where two ranges differ
template <typename iter1, typename iter2,
          typename value_type1=typename iter1::value_type, typename value_type2=typename iter2::value_type,
          typename predicate=std::function<bool(value_type1, value_type2)>>
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
    std::string http{"http"};
    auto match = utils::mismatch(http.cbegin(), http.cend(), start, end);
    if (match.first == http.cend()) {
        // Check http:// and https://
        auto checkStart = match.second;
        if (*checkStart == 's')
            ++checkStart;
        std::string protocol{"://"};
        auto match = utils::mismatch(protocol.cbegin(), protocol.cend(), checkStart, end);
        if (match.first == protocol.cend())
            return false;
    }
    return true;
}

}
}
