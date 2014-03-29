/**
 * Performs general tests on rewriteHTMl, just checking the input and output strings.
 * Doesn't care about blocks nor events triggered, just about the output.
 **/
#include "Rewriter.hpp"
#include "Rewriter_impl.hpp"
#include "Config.hpp"

#include <ostream>
#include <string>
#include <iterator>

#include <bandit/bandit.h>

using namespace cdnalizer;
using namespace bandit;

go_bandit([](){

    std::string output;

    using iterator = std::string::const_iterator;

    cdnalizer::Config cfg{
        {{"/images", "http://cdn.supa.ws/imgs"}}
    };
    std::string location = "/blog";

    RangeEvent<iterator> unchanged = [&](iterator start, iterator end) {
        auto to_output = std::back_inserter(output);
        std::copy(start, end, to_output);
        return end;
    };

    DataEvent newData = [&](std::string data) {
        output.append(data);
    };

    using namespace std::placeholders;
    auto doRewrite = [&](const std::string& input) {
        return cdnalizer::rewriteHTML(location, cfg, input.cbegin(), input.cend(), unchanged, newData);
    };

    before_each([&]() {
        output.clear();
    });

    describe("High level Rewrite HTML", [&](){
        it("1. Handles double quote attributes", [&](){
            std::string input(R"**(<img src="/images/a.gif">)**");
            doRewrite(input);
            AssertThat(output, Is().EqualTo(R"**(<img src="http://cdn.supa.ws/imgs/a.gif">)**"));
        });
        it("2. Handles single quote attributes", [&](){
            std::string input(R"**(<img src="/images/a.gif">)**");
            doRewrite(input);
            AssertThat(output, Is().EqualTo(R"**(<img src="http://cdn.supa.ws/imgs/a.gif">)**"));
        });
        it("3. Fails with double-single quote attributes", [&](){
            std::string input(R"**(<img src="/images/a.gif'>)**");
            doRewrite(input);
            AssertThat(output, Is().EqualTo(R"**(<img src="/images/a.gif'>)**"));
        });
        it("4. Fails with single-double quote attributes", [&](){
            std::string input(R"**(<img src='/images/a.gif">)**");
            doRewrite(input);
            AssertThat(output, Is().EqualTo(R"**(<img src='/images/a.gif">)**"));
        });
    });

});

int main(int argc, char** argv) {
    return bandit::run(argc, argv);
}
