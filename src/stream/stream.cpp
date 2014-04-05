#include "stream.hpp"
#include "../Rewriter.hpp"
#include "../Rewriter_impl.hpp"
#include "iterator.hpp"

namespace cdnalizer {
namespace stream {

struct Rewriter {
    std::ostream& output;
    std::ostream_iterator<char> out;

    // A callable nested class, to simlutae std::bind w/o creating a dependency on boost
    struct DataEvent : public std::unary_function<const std::string&, void> {
        Rewriter& rewriter;
        DataEvent(Rewriter& rewriter) : rewriter(rewriter) {}
        DataEvent(const DataEvent& other) : rewriter(other.rewriter) {}
        void operator ()(const std::string& data) { rewriter.output << data; };
    };
    DataEvent dataEvent;

    struct RangeEvent : public std::binary_function<Iterator&, Iterator&, Iterator> {
        Rewriter& rewriter;
        RangeEvent(Rewriter& rewriter) : rewriter(rewriter) {}
        RangeEvent(const RangeEvent& other) : rewriter(other.rewriter) {}
        Iterator operator ()(Iterator& a, Iterator& b) { std::copy(a, b, rewriter.out); return b; };
    };
    RangeEvent rangeEvent;

    // Event handlers
    Iterator noChange(Iterator& a, Iterator& b) { std::copy(a, b, out); return b; };
    void newData(const std::string& data) { output << data; };

    Rewriter(const std::string& location,
             const Config& config,
             std::istream& html,
             std::ostream& output) 
    : output(output), out(output), dataEvent(*this), rangeEvent(*this)
    {
       // Sort out our iterators
       Iterator in_start(html);
       Iterator in_end;

       Iterator done = cdnalizer::rewriteHTML<Iterator, char, RangeEvent, DataEvent>(
            location, config, in_start, in_end,
            rangeEvent, dataEvent);

        // Just ignore extra output - and push it out
        if (done != in_end)
            std::copy(done, in_end, out);
    }
};

//Iterator cdnalizer::rewriteHTML<Iterator, char, RangeEvent, DataEvent>(const& string, cdnalizer::Config const&, Iterator, Iterator, RangeEvent, DataEvent)'

void rewriteHTML(const std::string& location,
                 const Config& config,
                 std::istream& html,
                 std::ostream& output)
{
    Rewriter rewriter(location, config, html, output);
}

}
}
