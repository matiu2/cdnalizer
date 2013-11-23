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
 * To stop unnecssary creation of a buffer, through copying, the same iterator is returned with a 'lazy_inc' flag set.
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
    using buffer_type = std::vector<char_type>;
    using buffer_holder = buffer_type*;
    using buffer_offset = typename buffer_type::iterator;
private:
    stream_type& stream;
    char_type value;
    type* next = nullptr;
    type* prev = nullptr;
    buffer_holder buffer = nullptr;
    buffer_offset pos{};

    /// Creates a buffer and makes sure that all iterators in the chain have a copy
    void createBuffer() {
        assert(!buffer);
        buffer = new buffer_type;
        // To the end of the chain
        type* other = next;
        while (other) {
            assert(!other->buffer);
            other->buffer = buffer;
            other = other->next;
        }
        // To the beginning of the chain
        other = prev;
        while (other) {
            assert(!other->buffer);
            other->buffer = buffer;
            other = other->prev;
        }
    }

    /// Remove ourself from the chain
    void unlink() {
        // We're no longer in a chain, and have no use for a buffer
        buffer = nullptr;
        if (prev)
            prev->next = next;
        if (next)
            next->prev = prev;
    }
public:
    BaseIterator(stream_type& stream) : stream(stream) {
        stream.read(&value, 1);
    }
    BaseIterator(type& other) : stream(other.stream), value(other.value), next(&other), prev(other.prev), buffer(other.buffer)  {
        // Insert our selves in the chain
        other.prev = this;
        // Create a buffer if there isn't one
        if (!buffer)
            createBuffer();
    }
    ~BaseIterator() {
        // Clean up the buffer if we're the last one alive
        if (buffer && !prev && !next)
            delete buffer;
        else
            unlink();
    }
    char_type operator *() { return value; }
    char_type* operator ->() { return &value; }
    type& operator++() {
        // If we have no buffer, read from the stream
        if (!buffer)
            stream.read(&value, 1);
        // If we can read from the buffer, do it
        else if (pos != buffer_offset{})
            value = *(++pos);
        // If we're at the end of the buffer
        else {
            // Read from the stream
            stream.read(&value, 1);
            // Append the byte we read to the buffer
            buffer->push_back(value);
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
                prev = end;
                next = nullptr;
            }
            // Make any 'past the end' buffer pointers behind us, point to the last byte of the buffer
            type* past = prev;
            auto end = buffer->end();
            auto last = buffer->end() - 1;
            while (past) {
                if (past->pos == end)
                    past->pos = last;
                past = past->prev;
            }
        }
        return *this;
    }
    type operator++(int) {
        type result{*this};
        operator++();
        return result;
    }
};


using Iterator=BaseIterator<char>;

}
}
