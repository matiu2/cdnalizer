/** Iterator for going over Apache  APR_BUCKET_LISTS */

#include <stdexcept>
#include <cassert>

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
public:
    BucketWrapper(apr_bucket_brigade* bb, apr_bucket* bucket) : bb{bb}, bucket{bucket} {
        if (bb != nullptr)
            checkStatusCode(apr_bucket_read(bucket, &data, &length, APR_BLOCK_READ));
    }
    BucketWrapper(apr_bucket_brigade* bb) : BucketWrapper(bb, APR_BRIGADE_FIRST(bb)) {}
    BucketWrapper() : BucketWrapper(nullptr, nullptr) {} // We need to provide this to be a ForwardIterator
    /// Reperents the last bucket of any brigade
    const char* begin() const { return data; }
    const char* end() const { return data + length; }
    BucketWrapper next() const { return BucketWrapper(bb, APR_BUCKET_NEXT(bucket)); }
    /// Means we are the sentinel .. one past the end
    bool isLast() const { return (bb == nullptr) || (bucket == APR_BRIGADE_SENTINEL(bb)); }
    bool operator ==(const BucketWrapper& other) const {
        if (isLast() && other.isLast())
            return (bb == nullptr) || (other.bb == nullptr) || (bb == other.bb);
        return (bb == other.bb) && (bucket == other.bucket);
    }
    /// Splits the block in half
    void split(char* pos) {
        apr_bucket_split(block, pos);
        length = pos - data;
    }
};

// Forward iterator working on Apache bucket Brigades
struct Iterator : AbstractBlockIterator<char*, BucketWrapper, char>  {
    Iterator() = default;
    Iterator(const BucketWrapper& bucket, char* position={}) : AbstractBlockIterator(bucket, position) {}
    Iterator(const Iterator& other) = default;
    void split() { blocks.split(position); }
};

/// Convenience function to return the (one after the last) Iterator in a brigade
Iterator EndIterator(apr_bucket_brigade* bb) {
    return Iterator{BucketWrapper{bb, APR_BRIGADE_SENTINEL(bb)}, 0};
}

}
}
