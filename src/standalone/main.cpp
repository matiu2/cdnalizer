/** Standalone program reads in stdin .. outputs to stdout.
 *
 * Ports html resource references to cdn urls.
 */

#include "../stream/stream.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>

int main(int, char**) {
    cdnalizer::Container path2url;
    path2url.insert(std::make_pair("/images", "http://cdn.supa.ws/imgs"));
    cdnalizer::Config cfg(path2url);
    // Because cin doesn't allow you to hold to or re-use iterators, we'll just copy the whole of stdin to a tmp file for now
    //  TODO: Make a buffered, copyable, re-useable istream iterator
    std::fstream f("data.tmp");
    
    cdnalizer::stream::rewriteHTML("", cfg, std::cin, std::cout);
    std::cout.flush();
}
