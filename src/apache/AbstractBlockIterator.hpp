#pragma once
/** Iterator for going over Apache  APR_BUCKET_LISTS */

#include <iterator>

#include "utils.hpp"

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
    using parent_type=std::iterator<std::forward_iterator_tag, char>;
    using type=AbstractBlockIterator<SubIterator, Block, CharType>;
    using value_type=CharType;

    Block block;
    SubIterator position;

    AbstractBlockIterator() = default;
    AbstractBlockIterator(const Block& block, SubIterator position={}) : parent_type{}, block{block}, position{position ?  position : block.begin()} {}
    AbstractBlockIterator(const type& other) = default;
    type& operator =(const type& other) = default;
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
};

}
}
