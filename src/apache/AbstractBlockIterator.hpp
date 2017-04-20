#pragma once
/**
 * Iterator for going over Apache  APR_BUCKET_LISTS
 *
 * © Copyright 2014 Matthew Sherborne. All Rights Reserved.
 * License: Apache License, Version 2.0 (See LICENSE.txt)
 **/

#include <iterator>
#include <stdexcept>
#include <cassert>

#include <boost/hana/if.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/type.hpp>
#include <boost/hana/core/when.hpp>

namespace cdnalizer {
namespace apache {

/** Abstract Forward Iterator - for working on blocks of bytses
 *
 * The 'Block' type should support these functions
 *
 *  * SubIterator begin();
 *  * SubIterator end(); // One past the end
 *  * Block ++(); // Get the next block (prefix increment operator)
 *  * bool == // Is this block == to the next block..
 *
 */
template <typename SubIterator, typename Block>
struct AbstractBlockIterator
    : public std::iterator<std::forward_iterator_tag, char> {
  using parent_type = std::iterator<std::forward_iterator_tag, char>;
  using type = AbstractBlockIterator<SubIterator, Block>;
  using value_type = typename std::iterator_traits<SubIterator>::value_type;
  using reference = typename std::iterator_traits<SubIterator>::reference;
  using difference_type = typename std::iterator_traits<SubIterator>::difference_type;

  Block block;
  SubIterator position;

  AbstractBlockIterator() = default;
  AbstractBlockIterator(const Block &block, SubIterator position = {})
      : parent_type{}, block{block},
        position{position == SubIterator() ? position
                                                         : block.begin()} {}
  AbstractBlockIterator(const type &other) = default;
  type &operator=(const type &other) = default;
  reference operator*() const { return *position; }
  reference &operator*() { return *position; }
  reference operator->() const { return *position; }
  reference &operator->() { return *position; }
  type &operator++() {
    ++position;
    if (position == block.end()) {
      ++block;
      position = block.begin();
    }
    return *this;
  }
  type operator++(int) {
    type result(*this);
    operator++(*this);
    return result;
  }
  type &operator--() {
    if (position == block.begin()) {
      auto canDecBlock =
          boost::hana::is_valid([](auto &&b) -> decltype(--b) {});
      auto decBlock = boost::hana::if_(canDecBlock(block),
                                       [this](auto &block) {
                                         --block;
                                         assert(!block.isSentinel());
                                         position = block.end();
                                       },
                                       [this](auto &) {
                                         throw std::logic_error(
                                             "Can't decrement this iterator "
                                             "past the beginning of the "
                                             "block");
                                       });
      decBlock(block);
    }
    --position;
    return *this;
  }
  type operator--(int) {
    type result(*this);
    operator--(this);
    return result;
  }
  // Random iterator implementation
  type &operator+=(long count) {
    // Assuming base iterator is random access
    if (count > 0) {
      while (count != 0) {
        // How far til the end of the block ?
        long eob = block.end() - position;
        if (eob > count) {
          position += count;
          break;
        } else {
          // If our target is some blocks away, go to the next block and keep
          // searching
          count -= eob;
          ++block;
          position = block.begin();
        }
      }
      return *this;
    } else {
      return *this -= -count;
    }
  }
  type &operator-=(long count) {
    if (count > 0) {
      while (count != 0) {
        // How far til the beginning of the block
        long db = position - block.begin();
        if (db >= count) {
          // If the target is inside this block, just move there
          position -= count;
          break;
        } else {
          count -= db;
          --block;
          position = block.end();
        }
      }
    }
    return *this;
  }
  type operator+(long count) {
    type result(*this);
    return result += count;
  }
  type operator-(long count) {
    type result(*this);
    return result -= count;
  }
  difference_type operator-(const type& other) {
    if (block == other.block) {
      return position - other.position;
    } else {
      // Assume that other is less than ours
      auto copy = other;
      long count = 0;
      while (copy.block != block) {
        count += copy.block.end() - copy.position;
        ++copy.block;
        copy.position = copy.block.begin();
        if (copy.isSentinel())
          throw std::runtime_error("Couldn't find a distance between two iterators");
      }
      return count + (position - other.position);
    }
  }
  
  bool operator==(const type &other) const {
    if (block.isSentinel() && other.block.isSentinel())
      return true;
    else
      return (block == other.block) && (position == other.position);
  }
  bool operator!=(const type &other) const { return !(*this == other); }
  bool isAtStartOfBlock() const { return position == block.begin(); }
};
}
}
