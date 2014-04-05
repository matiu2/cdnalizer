/**
 * © Copyright 2014 Matthew Sherborne. All Rights Reserved.
 * License: Apache License, Version 2.0 (See LICENSE.txt)
 **/
#include "stream.hpp"
#include "../Rewriter.hpp"
#include "../Rewriter_impl.hpp"
#include "iterator.hpp"

namespace cdnalizer {
namespace stream {


void rewriteHTML(const std::string& location,
                 const Config& config,
                 std::istream& html,
                 std::ostream& output)
{
    // Sort out our iterators
    Iterator in_start(html);
    Iterator in_end{};

    std::ostream_iterator<char> out(output);

    // Event handlers
    auto noChange = [&out](Iterator a, Iterator b) { std::copy(a, b, out); return b; };
    auto newData = [&output](const std::string& data) { output << data; };
    
    // Parse the html
    auto done = cdnalizer::rewriteHTML<Iterator>(
        location, config, in_start, in_end,
        noChange, newData);

    // Just ignore extra output - and push it out
    if (done != in_end)
        std::copy(done, in_end, out);
}

}
}
