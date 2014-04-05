#pragma once
/**
 * Iterator for going over Apache  APR_BUCKET_LISTS
 *
 * © Copyright 2014 Matthew Sherborne. All Rights Reserved.
 * License: Apache License, Version 2.0 (See LICENSE.txt)
 **/

#include <iterator>


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
template <typename SubIterator, typename Block, typename CharType=typename SubIterator::value_type>
struct AbstractBlockIterator : public std::iterator<std::forward_iterator_tag, char> {
    typedef std::iterator<std::forward_iterator_tag, char> parent_type;
    typedef AbstractBlockIterator<SubIterator, Block, CharType> type;
    typedef CharType value_type;

    Block block;
    SubIterator position;

    AbstractBlockIterator() {}
    AbstractBlockIterator(const Block& block, SubIterator position=SubIterator()) : parent_type(), block(block), position(position ?  position : block.begin()) {}
    AbstractBlockIterator(const type& other) : block(other.block), position(other.position) {}
    type& operator =(const type& other) { block = other.block; position = other.position; return *this; }
    value_type operator *() const { return *position; }
    value_type& operator *() { return *position; }
    value_type operator ->() const { return *position; }
    value_type& operator ->() { return *position; }
    type& operator++() {
        ++position;
        if (position == block.end()) {
            ++block;
            position = block.begin();
        }
        return *this;
    }
    type operator++ (int) {
        type result(*this);
        operator++(*this);
        return result;
    }
    bool operator ==(const type& other) const {
        if (block.isSentinel() && other.block.isSentinel())
            return true;
        else
            return (block == other.block) && (position == other.position);
    }
    bool operator !=(const type& other) const { return !(*this == other); }
    bool isAtStartOfBlock() const { return position == block.begin(); }
    /// Splits the block at the current position.
    /// If succesful, we move to the beginnig of the next block after the split (data we point at stays the same)
    void split() {
        block.split(position);
        if (position == block.end()) {
            ++block;
            position = block.begin();
        }
    }
};

}
}
