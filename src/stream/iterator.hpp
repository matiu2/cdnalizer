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
#include <list>
#include <memory>
#include <iterator>
#include <cassert>

namespace cdnalizer {
namespace stream {


struct BadRead{};

/*  
 * TODO: Make a custom stream buf that wraps the streambuf of a stream when an iterator for that stream is created.
 * That way all iterators created on the stream can share the same buffer.
 */

template <typename CharType, typename stream_type=std::basic_istream<CharType>>
class BaseIterator : public std::iterator<std::forward_iterator_tag, CharType> {
public:
#ifdef HAVE_CPP11
    using char_type = CharType;
    using type = BaseIterator<char>;
    using buffer_type = std::vector<char_type>;
    using p_buffer_type = std::shared_ptr<std::vector<char_type>>;
    using buffer_offset = typename buffer_type::size_type;
#else
    typedef CharType char_type;
    typedef BaseIterator<char> type;
    typedef std::vector<char_type> buffer_type;
    typedef std::shared_ptr<std::vector<char_type>> p_buffer_type;
    typedef typename buffer_type::size_type buffer_offset;
#endif
private:
#ifdef HAVE_CPP11
    stream_type* stream = nullptr;
    char_type value = 0;
    p_buffer_type buffer;
    buffer_offset buf_pos{};
#else
    stream_type* stream;
    char_type value;
    p_buffer_type buffer;
    buffer_offset buf_pos;
#endif

    /// Returns true if we can't be incremented any more
    bool isEOF() const {
        if (!buffer && !stream)
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
    BaseIterator(const type& other) : stream(other.stream), value(other.value), buffer(other.buffer), buf_pos(other.buf_pos)  {
        // Create a buffer if there isn't one
        if (!stream)
            return;
        if (!buffer) {
            buffer = p_buffer_type(new buffer_type);
            const_cast<type&>(other).buffer = buffer;
        }
    }
    /// Constructor for a temporary, read-once and throw away object
    BaseIterator(char_type value) : value(value) {}
    /// Represents the end of a stream
    BaseIterator() {}
    char_type operator *() { return value; }
    char_type* operator ->() { return &value; }
    type& operator ++() {
        assert(stream); // Can't be called on read and throw away versions
        // If we have no buffer, read from the stream
        if (!buffer) {
            stream->read(&value, 1);
            if (stream->bad())
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
    template<typename OtherType>
    type& operator =(const OtherType& other) {
        stream = other.stream;
        value = other.value;
        buffer = other.buffer;
        buf_pos = other.buf_pos;
        return *this;
    }
};


#ifdef HAVE_CPP11
using Iterator=BaseIterator<char>;
#else
typedef BaseIterator<char> Iterator;
#endif

}
}
