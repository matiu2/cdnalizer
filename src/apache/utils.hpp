/** Utilitiies to help make the C more ++ier */
#pragma once

#include <cassert>
#include <stdexcept>
#include <memory>

extern "C" {
#include <apr_buckets.h>
}

namespace cdnalizer {
namespace apache {

#ifndef HAVE_CPP11
#define nullptr NULL
#endif

/** RAII guarded bucket brigade **/
class BrigadeGuard {
private:
    apr_bucket_brigade* bb;
    void cleanup() {
        if (bb != nullptr)
            apr_brigade_destroy(bb);
    }
    void moveFromOther(BrigadeGuard&& other) {
        assert(other.bb != bb); // That would be weird
        // Delete our current bb
        cleanup();
        // Steal our bro's brigade
        bb = other.bb;
        other.bb = nullptr;
    }
public:
    BrigadeGuard(apr_pool_t* pool, apr_bucket_alloc_t* list) {
        bb = apr_brigade_create(pool, list);
    }
    /// Move a bb from a bro
    BrigadeGuard(BrigadeGuard&& other) { moveFromOther(std::move(other)); }
    ~BrigadeGuard() { cleanup(); }
    BrigadeGuard& operator =(BrigadeGuard&& other) {
        moveFromOther(std::move(other));
        return *this;
    }
    apr_bucket_brigade& operator *() { return *bb; }
    apr_bucket_brigade& operator ->() { return *bb; }
    const apr_bucket_brigade& operator *() const { return *bb; }
    const apr_bucket_brigade& operator ->() const { return *bb; }
    operator apr_bucket_brigade*() { return bb; }
    operator apr_bucket_brigade&() { return *bb; }
    apr_bucket_brigade* brigade() { return bb; }
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

}
}
