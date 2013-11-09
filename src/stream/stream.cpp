#include "stream.hpp"
#include "../Rewriter.hpp"
#include "../Rewriter_impl.hpp"

#include <iterator>

namespace cdnalizer {
namespace stream {


void rewriteHTML(const std::string& location,
                 const Config& config,
                 std::istream& html,
                 std::ostream& output)
{
    // Sort out our iterators
    using iterator = std::istream_iterator<char>;
    iterator in_start(html);
    iterator in_end{};

    std::ostream_iterator<char> out(output);

    // Event handlers
    auto noChange = [&out](iterator a, iterator b) { std::copy(a, b, out); };
    auto newData = [&output](const std::string& data) { output << data; };
    
    // Parse the hctml
    auto done = cdnalizer::rewriteHTML<iterator>(
        location, config, in_start, in_end,
        noChange, newData);

    // Just ignore extra output - and push it out
    if (done != in_end)
        std::copy(done, in_end, out);
}

}
}
