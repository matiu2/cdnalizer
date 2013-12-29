#pragma once
#include <functional>

namespace cdnalizer {
namespace utils {

/// Our safe std::equal test for use with input iterators, with no way of knowing length
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
          typename value_type1=typename iter1::value_type, typename value_type2=typename iter2::value_type>
bool equal(iter1 first1, iter1 last1, iter2 first2, iter2 last2) {
    return equal(first1, last1, first2, last2, &default_pred<value_type1, value_type2>);
}

/// @return first position where two ranges differ
template <typename iter1, typename iter2>
std::pair<iter1, iter2> mismatch(iter1 first1, iter1 last1, iter2 first2, iter2 last2) {
    while ( (first1!=last1) && (first2!=last2) && (*first1==*first2) )
        { ++first1; ++first2; }
    return std::make_pair(first1,first2);
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

}
}
