#pragma once
/** Iterator for going over Apache  APR_BUCKET_LISTS */

#include <stdexcept>
#include <cassert>

#include "utils.hpp"

namespace cdnalizer {
namespace apache {

/** Abstract Forward Iterator - for working on blocks of bytses
 *
 * The 'Block' type should support these functions
 *
 *  * SubIterator begin();
 *  * SubIterator end(); // One past the end
 *  * Block& next();
 *  * bool isLast();
 *
 */
template <typename SubIterator, typename Block, typename CharType=typename SubIterator::value_type>
struct AbstractBlockIterator {
    using type=AbstractBlockIterator<SubIterator, Block, CharType>;
    using value_type=CharType;

    Block block;
    SubIterator position;

    AbstractBlockIterator() = default;
    AbstractBlockIterator(const Block& block, SubIterator position={}) : block{block}, position{position} {}
    AbstractBlockIterator(const type& other) = default;
    type& operator =(const type& other) = default;
    value_type operator *() const { return *position; }
    value_type& operator *() { return *position; }
    value_type operator ->() const { return *position; }
    value_type& operator ->() { return *position; }
    type& operator++() {
        position++;
        if (position == block.end()) {
            ++block;
            position = block.begin();
        }
        return *this;
    }
    type operator++ (int) {
        type result(*this);
        result++;
        return result;
    }
    bool isAtStartOfBlock() const { return position == block.begin(); }
};


/// Turns an apr status code into a c++ exception
class ApacheException : public std::runtime_error {
private:
    std::string static getMessage(apr_status_t code) {
        char data[256];
        apr_strerror(code, data, 256);
        return std::string(data);
    }
public:
    const apr_status_t code;
    ApacheException(apr_status_t code) 
        : std::runtime_error(getMessage(code)), code(code) {}
};

/// Throws an exception if the code is not APR_SUCCESS
void checkStatusCode(apr_status_t code) {
    if (code != APR_SUCCESS)
        throw ApacheException(code);
}

/// Wraps an APR bucket. Makes it useable in AbstractBlockIterator
class BucketWrapper {
private:
    apr_bucket_brigade* bb;
    apr_bucket* bucket;
    apr_size_t length;
    const char* data;
    /// Inits all our variables for the next bucket
    void init4NewBucket() {
        while (bucket) {
            if (APR_BUCKET_IS_EOS(bucket)) {
                // There is no data, this is the marker of the end of all data
                bucket = APR_BRIGADE_SENTINEL(bb);
                length = 0;
                data = nullptr;
                return;
            }
            else if (APR_BUCKET_IS_FLUSH(bucket)) {
                // TODO: Figure out how to implement flush(). One should create a temporary bb and move all buckets to it, then send that to the next handler.
                // The trouble is, however, that other iterators may point to the buckets getting flushed then get screwed up.
                // For now, just send the flush packet, even if it's out of order
                BrigadeGuard tmpbb; // Make a temporary brigade
                apr_bucket* next = APR_BUCKET_NEXT(bucket);
                APR_BRIGADE_REMOVE(bucket);
                APR_BRIGADE_INSERT_TAIL(bucket);
                bucket = next;
            }
            if (APR_BUCKET_IS_METADATA(bucket)) {
                // Just skip over the bucket; it'll get pushed along next flush
                bucket = APR_BUCKET_NEXT(bucket);
            }
        }
        assert(bucket);
        // Get the data out of the bucket
        checkStatusCode(apr_bucket_read(bucket, &data, &length, APR_BLOCK_READ));
    }
public:
    BucketWrapper(apr_bucket_brigade* bb, apr_bucket* bucket) : bb{bb}, bucket{bucket} {
        if (bb != nullptr)
            init4NewBucket();
    }
    BucketWrapper(apr_bucket_brigade* bb) : BucketWrapper(bb, APR_BRIGADE_FIRST(bb)) {}
    BucketWrapper() : BucketWrapper(nullptr, nullptr) {} // We need to provide this to be a ForwardIterator
    /// Reperents the last bucket of any brigade
    const char* begin() const { return data; }
    const char* end() const { return data + length; }
    BucketWrapper next() const { return BucketWrapper(bb, APR_BUCKET_NEXT(bucket)); }
    /// Means we are the sentinel .. one past the end
    bool isEnd() const { return (bb == nullptr) || (bucket == APR_BRIGADE_SENTINEL(bb)); }
    bool operator ==(const BucketWrapper& other) const {
        if (isEnd() && other.isEnd())
            return (bb == nullptr) || (other.bb == nullptr) || (bb == other.bb);
        return (bb == other.bb) && (bucket == other.bucket);
    }
    /// Splits the block at @pos
    void split(char* pos) {
        if ((pos != data) && (pos != data + length)) {
            apr_bucket_split(block, pos);
            length = pos - data;
        }
    }
    operator ++() { bucket = APR_BUCKET_NEXT(bucket); }
    apr_bucket* bucket() {
        return bucket;
    }
};

// Forward iterator working on Apache bucket Brigades
struct Iterator : AbstractBlockIterator<char*, BucketWrapper, char>  {
    Iterator() = default;
    Iterator(const BucketWrapper& bucket, char* position={}) : AbstractBlockIterator(bucket, position) {}
    Iterator(const Iterator& other) = default;
    /// Splits the block at the current position.
    void split() { block.split(position); }
};

/// Convenience function to return the (one after the last) Iterator in a brigade
Iterator EndIterator(apr_bucket_brigade* bb) {
    return Iterator{BucketWrapper{bb, APR_BRIGADE_SENTINEL(bb)}, 0};
}

}
}
