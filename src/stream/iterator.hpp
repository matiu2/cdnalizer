#pragma once
/** An istream iterator that allows a small amount of stream re-use through buffering.
 *
 * Reads an istream, and allows one to read from an iterator twice.
 *
 * It allows iterator assignment through holding a buffer.
 *
 * Related iterators (created by copying) remember each other through a linked list
 */

#include <istream>
#include <vector>
#include <memory>
#include <iterator>
#include <cassert>

namespace cdnalizer {
namespace stream {

template <typename char_type, typename stream_type=std::basic_istream<char_type>>
class BaseIterator {
public:
    using value_type = char_type;
    using type = BaseIterator<char_type>;
    using buffer_type = std::unique_ptr<std::vector<char_type>>;
private:
    stream_type& stream;
    bool haveRead = false;
    char_type value;
    buffer_type buffer;
    type* next = nullptr;
    type* prev = nullptr;
    void getValue() {
        if (!haveRead) {
            stream.read(&value, 1);
            if (prev && prev->buffer)
                prev->buffer->push_back(value);
            haveRead = true;
        }
    }
    /// Gives our buffer to another iterator
    void passBuffer(type* other) {
        assert(other);
        assert(this->buffer);
        if (prev->buffer) {
            prev->buffer.append(*buffer);
            std::copy(buffer->cbegin(), buffer->cend(), std::back_insert_iterator<buffer_type>(*(prev->buffer)));
            buffer.reset();
        } else {
            prev->buffer = std::move(buffer);
        }
    }
    /// Remove ourself from the chain, give our buffer to our previous element
    void unlink() {
        if (buffer) {
            if (prev)
                passBuffer(prev);
            else if (next)
                passBuffer(next);
        }
        if (prev)
            prev->next = next;
        if (next)
            next->prev = prev;
    }
public:
    BaseIterator(stream_type& stream) : stream(stream) {}
    BaseIterator(type& other) : stream(other.stream), haveRead(other.haveRead), value(other.value), next(&other) {
        if (!haveRead) {
            // If the other one is in the pre-lazy-read state, do the read so we have good data.
            other.getValue();
            value = other.value;
            haveRead = other.haveRead;
        }
        if (other.prev)
            prev = other.prev;
        other.prev = this;
    }
    char_type operator *() {
        getValue();
        return value;
    }
    char_type* operator ->() {
        getValue();
        return &value;
    }
    void operator++() {
        if (haveRead)
            haveRead = false;
        else
            getValue();
        if (prev && prev->buffer)
            prev->buffer->push_back(value);
    }
    type operator++(int) {
        type result{*this};
        operator++();
        return result;
    }
    char_type& operator =(BaseIterator<char_type>* other) {
        unlink();
        stream = other->stream;
        haveRead = other->haveRead;
        value = other->value;
        next = other;
        prev = other->prev;
        other->prev = this;
        if (other->buffer) {
            assert(!prev);
            buffer = std::move(other->buffer); // Only the number one in the chain holds the buffer
        }
        return *this;
    }
};

using Iterator=BaseIterator<char>;

}
}
