#pragma once
/**
 * Iterator for going over Apache  APR_BUCKET_LISTS
 *
 * © Copyright 2014 Matthew Sherborne. All Rights Reserved.
 * License: Apache License, Version 2.0 (See LICENSE.txt)
 *
 **/

#include <stdexcept>
#include <cassert>

#include "AbstractBlockIterator.hpp"
#include "utils.hpp"

extern "C" {
#include <apr_buckets.h>
}

namespace cdnalizer {
namespace apache {

/// Wraps an APR bucket. Makes it useable in AbstractBlockIterator
template <typename FlushHandler>
class BucketWrapper {
private:
    apr_bucket_brigade* bb;
    FlushHandler onFlush;
    apr_bucket* _bucket;
    apr_size_t length;
    const char* data;
    /// Inits all our variables for the next bucket
    void init4NewBucket() {
        apr_bucket* sentinel = APR_BRIGADE_SENTINEL(bb);
        // Skip over non-data buckets
        while (_bucket != sentinel) {
            if (APR_BUCKET_IS_METADATA(_bucket)) {
                // Just skip over the bucket; it'll get pushed along next flush
            } else if (APR_BUCKET_IS_FLUSH(_bucket)) {
                // We should send our completed work on to the next filter
                if (onFlush)
                    checkStatusCode(onFlush());
            } else {
                // Any other bucket is not skippable
                break;
            }
            _bucket = APR_BUCKET_NEXT(_bucket);
        }
        // Now we've skipped all the skippable buckets .. see what we can do
        if (APR_BUCKET_IS_EOS(_bucket)) {
            // There is no data, this is the marker of the end of all data for this request
            // We must send everything over to the next filter
            _bucket = APR_BRIGADE_SENTINEL(bb);
            length = 0;
            data = NULL;
            return;
        } else {
            // Get the data out of the bucket if there is any
            if (_bucket != sentinel)
                checkStatusCode(apr_bucket_read(_bucket, &data, &length, APR_BLOCK_READ));
        }
    }
public:
    BucketWrapper(apr_bucket_brigade* bb, FlushHandler onFlush, apr_bucket* bucket)
    : bb(bb), onFlush(onFlush), _bucket(bucket), data(NULL)
    {
        if (bb != NULL)
            init4NewBucket();
    }
    BucketWrapper(apr_bucket_brigade* bb, FlushHandler onFlush) : bb(bb), onFlush(onFlush), _bucket(APR_BRIGADE_FIRST(bb)) { init4NewBucket(); }
    BucketWrapper() :  bb(bb), onFlush(NULL), _bucket(NULL), data(NULL) { init4NewBucket(); }
    const char* begin() const { return data; }
    const char* end() const { return data + length; }
    /// Means we are the sentinel bucket; one past the end .. there is no more data to process
    bool isSentinel() const { return (bb == NULL) || (_bucket == APR_BRIGADE_SENTINEL(bb)); }
    bool operator ==(const BucketWrapper& other) const {
        if (isSentinel() && other.isSentinel())
            return true; // Assumes all iterators being compared are from the same bucket brigade.
        return (bb == other.bb) && (_bucket == other._bucket);
    }
    /** Splits the bucket at @pos.
     *  If it fails, it leaves us unchanged.
     *  If it succeeds it leaves us at the beginning of the next bucket after the split
     */
    void split(const char* pos) const {
        if (isSentinel())
            return; // Can't split the one-after-last bucket
        if ((pos != data) && (pos != data + length)) {
            apr_bucket_split(_bucket, pos-data);
            checkStatusCode(apr_bucket_read(_bucket, const_cast<const char**>(&data), const_cast<apr_size_t*>(&length), APR_BLOCK_READ));
        }
    }
    // Point to the next bucket
    BucketWrapper& operator ++() {
        assert(!isSentinel()); // Why would you try to go past the end of the sentinel!?
        _bucket = APR_BUCKET_NEXT(_bucket);
        init4NewBucket();
        return *this;
    }
    apr_bucket* bucket() const { return _bucket; }
};

// Forward iterator working on Apache bucket Brigades
template <typename FlushHandler>
struct Iterator : AbstractBlockIterator<const char*, BucketWrapper<FlushHandler>, const char>  {
    typedef AbstractBlockIterator<const char*, BucketWrapper<FlushHandler>, const char> Base;
    Iterator() : Base() {}
    Iterator(apr_bucket_brigade* bb, FlushHandler onFlush, apr_bucket* bucket, char* position=NULL)
        : Base(BucketWrapper<FlushHandler>(bb, onFlush, bucket), position) {}
    Iterator(apr_bucket_brigade* bb, FlushHandler onFlush, char* position=NULL) 
        : Base(BucketWrapper<FlushHandler>(bb, onFlush), position) {}
    Iterator(const Iterator& other) : Base(other) {}
    Iterator& operator=(const Iterator& other) {
        Base::operator=(other);
        return *this;
    }
    apr_bucket* bucket() const { return Base::block.bucket(); }
    Iterator& operator++() { return static_cast<Iterator&>(Base::operator++()); }
    Iterator operator++ (int) {
        Iterator result(*this);
        operator++();
        return result;
    }
    char operator *() const { return Base::operator*(); }
};

/// Convenience function to return the (one after the last) Iterator in a brigade
template <typename FlushHandler>
Iterator<FlushHandler> EndIterator(apr_bucket_brigade* bb) {
    return Iterator<FlushHandler>(bb, FlushHandler(), APR_BRIGADE_SENTINEL(bb), 0);
}

}
}
