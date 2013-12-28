#pragma once
/** Iterator for going over Apache  APR_BUCKET_LISTS */

#include <stdexcept>
#include <cassert>

#include "utils.hpp"

extern "C" {
#include <apr_buckets.h>
}

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
        ++position;
        if (position == block.end()) {
            ++block;
            position = block.begin();
        }
        return *this;
    }
    type operator++ (int) {
        type result(*this);
        ++result;
        return result;
    }
    bool operator ==(const type& other) { return block == other.block; }
    bool operator !=(const type& other) { return !(*this == other); }
    bool isAtStartOfBlock() const { return position == block.begin(); }
};

/// Wraps an APR bucket. Makes it useable in AbstractBlockIterator
class BucketWrapper {
public:
    typedef std::function<apr_status_t()> FlushHandler;
private:
    apr_bucket_brigade* bb;
    FlushHandler onFlush;
    apr_bucket* bucket;
    apr_size_t length;
    const char* data;
    /// Inits all our variables for the next bucket
    void init4NewBucket() {
        apr_bucket* sentinel = APR_BRIGADE_SENTINEL(bb);
        while (bucket != sentinel) {
            if (APR_BUCKET_IS_EOS(bucket)) {
                // There is no data, this is the marker of the end of all data
                bucket = APR_BRIGADE_SENTINEL(bb);
                length = 0;
                data = nullptr;
                return;
            }
            else if (APR_BUCKET_IS_FLUSH(bucket)) {
                // We should send our completed work on to the next filter
                if (onFlush)
                    checkStatusCode(onFlush());
            }
            if (APR_BUCKET_IS_METADATA(bucket)) {
                // Just skip over the bucket; it'll get pushed along next flush
                bucket = APR_BUCKET_NEXT(bucket);
            }
        }
        assert(bucket != sentinel);
        assert(bucket);
        // Get the data out of the bucket
        checkStatusCode(apr_bucket_read(bucket, &data, &length, APR_BLOCK_READ));
    }
public:
    BucketWrapper(apr_bucket_brigade* bb, FlushHandler onFlush, apr_bucket* bucket)
    : bb{bb}, onFlush{onFlush}, bucket{bucket}
    {
        if (bb != nullptr)
            init4NewBucket();
    }
    BucketWrapper(apr_bucket_brigade* bb, FlushHandler onFlush) : BucketWrapper(bb, onFlush, APR_BRIGADE_FIRST(bb)) {}
    BucketWrapper() : BucketWrapper(nullptr, nullptr, nullptr) {} // We need to provide this to be a ForwardIterator
    /// Reperents the last bucket of any brigade
    const char* begin() const { return data; }
    const char* end() const { return data + length; }
    /// Means we are the sentinel .. one past the end
    bool isEnd() const { return (bb == nullptr) || (bucket == APR_BRIGADE_SENTINEL(bb)); }
    bool operator ==(const BucketWrapper& other) const {
        if (isEnd() && other.isEnd())
            return (bb == nullptr) || (other.bb == nullptr) || (bb == other.bb);
        return (bb == other.bb) && (bucket == other.bucket);
    }
    /// Splits the bucket at @pos
    apr_bucket* split(const char* pos) {
        if ((pos != data) && (pos != data + length)) {
            apr_bucket_split(bucket, pos-data);
            checkStatusCode(apr_bucket_read(bucket, &data, &length, APR_BLOCK_READ));
        }
        return bucket;
    }
    // Point to the next bucket
    BucketWrapper& operator ++() {
        bucket = APR_BUCKET_NEXT(bucket);
        init4NewBucket();
        return *this;
    }
};

// Forward iterator working on Apache bucket Brigades
struct Iterator : AbstractBlockIterator<const char*, BucketWrapper, char>  {
    Iterator() = default;
    Iterator(apr_bucket_brigade* bb, BucketWrapper::FlushHandler onFlush, apr_bucket* bucket, char* position={}) : AbstractBlockIterator({bb, onFlush, bucket}, position) {}
    Iterator(apr_bucket_brigade* bb, BucketWrapper::FlushHandler onFlush, char* position={}) : AbstractBlockIterator({bb, onFlush}, position) {}
    Iterator(const Iterator& other) = default;
    /// Splits the block at the current position.
    apr_bucket* split() { return block.split(position); }
};

/// Convenience function to return the (one after the last) Iterator in a brigade
Iterator EndIterator(apr_bucket_brigade* bb) {
    return Iterator{bb, BucketWrapper::FlushHandler{}, APR_BRIGADE_SENTINEL(bb), 0};
}

}
}
