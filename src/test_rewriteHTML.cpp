
#include <bandit/bandit.h>
#include "Rewriter.hpp"
#include "Rewriter_impl.hpp"
#include "Config.hpp"

using namespace bandit;
using namespace cdnalizer;

go_bandit([](){
    using Iterator = std::string::const_iterator;
    struct SequencedIteratorPair {
        size_t sequence;
        Iterator start;
        Iterator end;
    };
    using SequencedNewData = std::pair<size_t, std::string>;

    size_t sequence;
    std::vector<SequencedIteratorPair> unchanged_blocks;
    std::vector<SequencedNewData> new_blocks;
    std::string location = "/blog";

    cdnalizer::Config cfg{
        {{"/images", "http://cdn.supa.ws/imgs"}}
    };

    RangeEvent<Iterator> unchanged = [&](Iterator start, Iterator end) {
        unchanged_blocks.push_back(SequencedIteratorPair{sequence++, start, end});
    };

    DataEvent newData = [&](std::string data) {
        new_blocks.push_back(SequencedNewData(sequence++, data));
    };

    using namespace std::placeholders;
    auto doRewrite = std::bind(cdnalizer::rewriteHTML<Iterator>, location, cfg, _1, _2, unchanged, newData);
    //auto doRewrite = [&](Iterator start, Iterator end) { cdnalizer::rewriteHTML(location, cfg, start, end, unchanged, newData); };

    before_each([&]() {
        sequence = 1;
        unchanged_blocks.clear();
        new_blocks.clear();
    });

    // Returns true if nothing is changed after running doRewrite
    auto ensureNoChange = [&](const std::string& data) {
        Iterator end = doRewrite(data.cbegin(), data.cend());
        AssertThat(end, Is().EqualTo(data.cend()));
        AssertThat(unchanged_blocks.size(), Is().EqualTo((size_t)1));
        auto block = unchanged_blocks.at(0);
        AssertThat(block.sequence, Is().EqualTo((size_t)1));
        AssertThat(block.start, Is().EqualTo(data.cbegin()));
        AssertThat(block.end, Is().EqualTo(data.cend()));
    };

    describe("Simple Rewrite HTML", [&](){

        it("Returns unchanged when there are no tags", [&](){
            const std::string data{"There are no tags here"};
            ensureNoChange(data);
        });

        it("Returns unchanged when we don't find the tags in the library", [&](){
            const std::string data{R"**("This is <some funny="tag">that</some> <we dont="care"/> about")**"};
            ensureNoChange(data);
        });

        it("Ignores attributes we don't care about", [&](){
            const std::string data{R"**("<a src="/images/a.gif"><img href="/images/bad.link" />bad attribute name</a>")**"};
            ensureNoChange(data);
        });

    });
});

int main(int argc, char** argv) {
    bandit::run(argc, argv);
}
