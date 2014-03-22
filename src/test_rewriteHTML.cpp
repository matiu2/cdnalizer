#include "Rewriter.hpp"
#ifdef HAVE_CPP11
#include "Rewriter_impl.hpp"
#else
#include "Rewriter_impl_old.hpp"
#endif
#include "Config.hpp"

#include <ostream>

using namespace cdnalizer;

// Some custom types
using Iterator = std::string::const_iterator;
struct SequencedIteratorPair {
    size_t sequence;
    Iterator start;
    Iterator end;
};
using SequencedNewData = std::pair<int, std::string>;

// Nice ways to print some of the types we're working with

namespace std {

ostream& operator<<(ostream& stream, const SequencedIteratorPair& p) {
    string s;
    copy(p.start, p.end, back_inserter(s));
    stream << "SI - i(" << p.sequence << ") sz(" << p.end-p.start << ") data: --" << s << "--";
    return stream;
}

ostream& operator<<(ostream& stream, const SequencedNewData& n) {
    stream << "Sequenced New Data - sequence(" << n.first << ") length(" << n.second.length() << ") data: --" << n.second << "--";
    return stream;
}

ostream& operator<<(ostream& stream, const vector<SequencedIteratorPair>& v) {
    stream << "Vector of Iterator Pairs: Length(" << v.size() << ")";
    for(const SequencedIteratorPair& p : v)
        stream << p << endl;
    return stream;
}

ostream& operator<<(ostream& stream, const vector<SequencedNewData>& v) {
    stream << "Vector of new datas: Length(" << v.size() << ")";
    for(const SequencedNewData& p : v)
        stream << p << endl;
    return stream;
}

}

// Custom Operators

bool operator ==(const SequencedIteratorPair& lhs, const SequencedIteratorPair& rhs) {
    return (lhs.sequence == rhs.sequence) &&
           (lhs.start == rhs.start) &&
           (lhs.end == rhs.end);
}

// Acutal tests
#include <bandit/bandit.h>
using namespace bandit;

go_bandit([](){

    size_t sequence;
    std::vector<SequencedIteratorPair> unchanged_blocks;
    std::vector<SequencedNewData> new_blocks;
    std::string location = "/blog";

    cdnalizer::Config cfg{
        {{"/images", "http://cdn.supa.ws/imgs"}}
    };

    RangeEvent<Iterator> unchanged = [&](Iterator start, Iterator end) {
        unchanged_blocks.push_back(SequencedIteratorPair{sequence++, start, end});
        return end;
    };

    DataEvent newData = [&](std::string data) {
        new_blocks.push_back(SequencedNewData(sequence++, data));
    };

    using namespace std::placeholders;
    auto doRewrite = std::bind(cdnalizer::rewriteHTML<Iterator>, location, _3, _1, _2, unchanged, newData);
    //auto doRewrite = [&](Iterator start, Iterator end) { cdnalizer::rewriteHTML(location, cfg, start, end, unchanged, newData); };

    before_each([&]() {
        sequence = 1;
        unchanged_blocks.clear();
        new_blocks.clear();
    });

    // Returns true if nothing is changed after running doRewrite
    auto ensureNoChange = [&](const std::string& data) {
        Iterator end = doRewrite(data.cbegin(), data.cend(), cfg);
        AssertThat(end, Is().EqualTo(data.cend()));
        AssertThat(unchanged_blocks, HasLength(1));
        auto block = unchanged_blocks.at(0);
        AssertThat(block.sequence, Equals((size_t)1));
        AssertThat(block.start, Is().EqualTo(data.cbegin()));
        AssertThat(block.end, Is().EqualTo(data.cend()));
    };

    describe("Simple Rewrite HTML", [&](){

        it("1. Returns unchanged when there are no tags", [&](){
            const std::string data{"There are no tags here"};
            ensureNoChange(data);
        });

        it("2. Returns unchanged when we don't find the tags in the library", [&](){
            const std::string data{R"**(This is <some funny="tag">that</some> <we dont="care"/> about)**"};
            ensureNoChange(data);
        });

        it("3. Ignores attributes we don't care about", [&](){
            const std::string data{R"**(<a src="/images/a.gif"><img href="/images/bad.link" />bad attribute name</a>)**"};
            ensureNoChange(data);
        });

        it("4. Ignores an attribute we care about, but has the wrong value ", [&](){
            const std::string data{R"**(<a href="/not/images/a.gif"><img href="/images/bad.link" />bad attribute name</a>)**"};
            ensureNoChange(data);
        });

        it("5. Ignores when location breaks our match", [&](){
            const std::string data{R"**(<a href="images/a.gif"><img src="/images/bad.link" />Bad location</a>)**"};
            cfg.addPath("/blog2/images", "http://cdn.supa.ws/blog2/imags");
            Iterator end = doRewrite(data.cbegin(), data.cend(), cfg);
            AssertThat(end, Is().EqualTo(data.cend()));
            AssertThat(unchanged_blocks, HasLength(2));

            // Unchanged: "<a href="images/a.gif"><img src="
            auto block = unchanged_blocks.at(0);
            AssertThat(block, Equals(SequencedIteratorPair{1, data.cbegin(), data.cbegin()+33}));

            // New Data: "http://cdn.supa.ws/imgs"
            AssertThat(new_blocks, HasLength(1));
            auto new_block = new_blocks.at(0);
            AssertThat(new_block, Equals(SequencedNewData{2, "http://cdn.supa.ws/imgs"}));

            // Unchanged: '/bad.link" />Bad location</a>'
            block = unchanged_blocks.at(1);
            AssertThat(block, Equals(SequencedIteratorPair{3, data.cbegin()+40, data.cend()}));
        });

        it("6. Works out location correctly", [&](){
            const std::string data{R"**(<a href="images/a.gif"><img src="/images/bad.link" />Bad location</a>)**"};

            // location is '/blog/', so 'images/a.gif' should be interpreted as '/blog/images/a.gif'
            cfg.addPath("/blog/images", "http://cdn.supa.ws/blog/imags");
            Iterator end = doRewrite(data.cbegin(), data.cend(), cfg);
            AssertThat(end, Is().EqualTo(data.cend()));
            AssertThat(unchanged_blocks, HasLength(3));

            // Unchanged: "<a href="
            auto block = unchanged_blocks.at(0);
            AssertThat(block, Equals(SequencedIteratorPair{1, data.cbegin(), data.cbegin()+9}));

            // New Data: "http://cdn.supa.ws/blog/imags"
            AssertThat(new_blocks, HasLength(2));
            SequencedNewData new_block = new_blocks.at(0);
            AssertThat(new_block, Equals(SequencedNewData{2, "http://cdn.supa.ws/blog/imags"}));

            // Unchanged: '/a.gif"><img src='
            block = unchanged_blocks.at(1);
            AssertThat(block, Equals(SequencedIteratorPair{3, data.cbegin()+15, data.cbegin()+33}));

            // New Data: "http://cdn.supa.ws/imgs"
            new_block = new_blocks.at(1);
            SequencedNewData expected = SequencedNewData{4, "http://cdn.supa.ws/imgs"};
            AssertThat(new_block, Equals(expected));

            // Unchanged: '/bad.link" />Bad location</a>'
            block = unchanged_blocks.at(2);
            AssertThat(block, Equals(SequencedIteratorPair{5, data.cbegin()+40, data.cend()}));
        });
    });
});

int main(int argc, char** argv) {
    return bandit::run(argc, argv);
}
