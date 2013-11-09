/** Standalone program reads in stdin .. outputs to stdout.
 *
 * Ports html resource references to cdn urls.
 */

#include "../stream/stream.hpp"

#include <iostream>

int main(int, char**) {
    cdnalizer::Config cfg{
    {{"/images", "http://cdn.supa.ws/imgs"}}
    };
    cdnalizer::stream::rewriteHTML("", cfg, std::cin, std::cout);
    std::cout.flush();
}
