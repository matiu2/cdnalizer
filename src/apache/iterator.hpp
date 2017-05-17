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

// Forward iterator working on Apache bucket Brigades
struct Iterator : AbstractBlockIterator<const char*, BucketWrapper>  {
    using Base = AbstractBlockIterator<const char*, BucketWrapper>;
    Iterator() = default;
    Iterator(apr_bucket_brigade *bb, BucketWrapper::FlushHandler onFlush,
             apr_bucket *bucket, char *position = {})
        : AbstractBlockIterator({bb, onFlush, bucket}, position) {}
    Iterator(apr_bucket_brigade *bb, BucketWrapper::FlushHandler onFlush)
        : AbstractBlockIterator({bb, onFlush}) {}
    Iterator(apr_bucket_brigade *bb, BucketWrapper::FlushHandler onFlush,
             char *position)
        : AbstractBlockIterator({bb, onFlush}, position) {}
    Iterator(const Iterator& other) = default;
    /// Splits the block at the current position.
    /// If succesful, we move to the beginnig of the next block after the split (data we point at stays the same)
    void split() {
        block.split(position);
        if (position == block.end()) {
            ++block;
            position = block.begin();
        }
    }
    apr_bucket* bucket() const { return block.bucket(); }
    Iterator& operator++() { return static_cast<Iterator&>(Base::operator++()); }
    Iterator operator++ (int) {
        Iterator result(*this);
        operator++();
        return result;
    }
};

/// Convenience function to return the (one after the last) Iterator in a brigade
Iterator EndIterator(apr_bucket_brigade* bb) {
    return Iterator{bb, BucketWrapper::FlushHandler{}, APR_BRIGADE_SENTINEL(bb), 0};
}

}
}
