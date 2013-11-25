#pragma once
/** An istream iterator that allows a small amount of stream re-use through buffering.
 *
 * Reads an istream, and allows one to read from an iterator twice.
 *
 * It allows iterator assignment through holding a buffer.
 *
 * Related iterators (created by copying) remember each other through a linked list
 *
 * # How the buffering works
 *
 * 1. A single (uncoupled) iterator has no buffer, for efficiency reasons
 * 2. A chain of (coupled) iterators all share the same pointer to the same buffer
 * 3. Buffer is created on the 1st copy or assignment
 * 
 * 
 * ## When an iterator reads the next byte
 *
 * 1. If it can it'll read from the buffer
 * 2. If it can't
 * 2.1. It'll read from the stream
 * 2.2. It'll append the byte it read to the buffer
 * 2.3. It'll become the head of the chain
 * 2.4. It'll make any 'past the end' buffer pointers behind it, point to the last byte of the buffer
 *
 * ## When an iterator is created from a stream
 *
 * 1. It'll immediately consume the 1st byte of the stream into 'value' (no lazy reading)
 *
 * ## When my_it++ (post decrement operator) is called
 *
 */

#include <istream>
#include <vector>
#include <memory>
#include <iterator>
#include <cassert>

namespace cdnalizer {
namespace stream {


struct BadRead{};


template <typename char_type, typename stream_type=std::basic_istream<char_type>>
class BaseIterator : public std::iterator<std::forward_iterator_tag, char_type> {
public:
    using type = BaseIterator<char_type>;
    using buffer_type = std::vector<char_type>;
    using buffer_holder = buffer_type*;
    using buffer_offset = typename buffer_type::size_type;
private:
    stream_type* stream = nullptr;
    char_type value = 0;
    mutable type* next = nullptr;
    mutable type* prev = nullptr;
    buffer_holder buffer = nullptr;
    buffer_offset buf_pos{};

    /// Creates a buffer and makes sure that all iterators in the chain have a copy
    void createBuffer() {
        assert(!buffer);
        buffer = new buffer_type;
        buf_pos = buffer->size();
        // To the end of the chain
        type* other = next;
        while (other) {
            assert(!other->buffer);
            other->buffer = buffer;
            other->buf_pos = buf_pos;
            other = other->next;
        }
        // To the beginning of the chain
        other = prev;
        while (other) {
            assert(!other->buffer);
            other->buffer = buffer;
            other->buf_pos = buf_pos;
            other = other->prev;
        }
    }

    /// Remove ourself from the chain
    void unlink() {
        // We're no longer in a chain, and have no use for a buffer
        if (prev)
            prev->next = next;
        if (next)
            next->prev = prev;
    }

    /// Returns true if we can't be incremented any more
    bool isEOF() const {
        if (!buffer && !stream && !next && !prev)
            return true;
        if (stream && stream->eof())
            return true;
        if (!buffer && !stream)
            return true;
        return false;
    }

public:
    BaseIterator(stream_type& stream) : stream(&stream) {
        this->stream->read(&value, 1);
        if (this->stream->bad())
            throw BadRead();
    }
    BaseIterator(const type& other) : stream(other.stream), value(other.value), next(&const_cast<type&>(other)), prev(other.prev), buffer(other.buffer)  {
        // Insert our selves in the chain
        other.prev = this;
        // Create a buffer if there isn't one
        if (stream && !buffer)
            createBuffer();
    }
    /// Constructor for a temporary, read-once and throw away object
    BaseIterator(char_type value) : value(value) {}
    /// Represents the end of a stream
    BaseIterator() {}
    ~BaseIterator() {
        // Clean up the buffer if we're the last one alive
        if (buffer && !prev && !next)
            delete buffer;
        else {
            buffer = nullptr;
            unlink();
        }
    }
    char_type operator *() { return value; }
    char_type* operator ->() { return &value; }
    type& operator++() {
        assert(stream); // Can't be called on read and throw away versions
        // If we have no buffer, read from the stream
        if (!buffer) {
            stream->read(&value, 1);
            if (this->stream->bad())
                throw BadRead();
        }
        // If we can read from the buffer, do it
        else if (buf_pos != buffer->size())
            value = buffer->at(buf_pos++);
        // If we're at the end of the buffer
        else {
            // Read from the stream
            stream->read(&value, 1);
            // Append the byte we read to the buffer
            buffer->push_back(value);
            buf_pos = buffer->size();
            // Become the head of the chain
            if (next) {
                // Remove ourselves from the end of the chain
                if (prev)
                    prev->next = next;
                next->prev = prev;
                // Find the end of the chain
                type* end = next;
                while (end->next)
                    end = end->next;
                // Become the end
                end->next = this;
                unlink();
                prev = end;
                next = nullptr;
            }
        }
        return *this;
    }
    type operator++(int) {
        assert(stream); // Can't be called on read and throw away versions
        type result{value}; // Create a read-and-throw instance to return
        operator++();
        return result;
    }
    /// Only useful for checking if this iterator is at the end of the data
    bool operator==(const type& other) const {
        if (isEOF())
            // Both are eof
            return other.isEOF();
        else if (buffer && other.buffer)
            // Same buffer and same pos
            return (buffer == other.buffer) && (buf_pos == other.buf_pos);
        else if (stream && (stream == other.stream) && !buffer && !other.buffer && (value == other.value))
            // Same stream, and neither has a buffer, and both have the same value
            return true;
        return false;
    }
    bool operator!=(const type& other) const {
        return !operator==(other);
    }
};


using Iterator=BaseIterator<char>;

}
}
