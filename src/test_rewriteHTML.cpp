/**
 * Tests RewriteHTML at a low level. 
 * Ensure's events are triggered correctly and that blocks are processed correctly.
 **/
#include "Config.hpp"
#include "Rewriter.hpp"
#include "Rewriter_impl.hpp"

#include <bandit/bandit.h>
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
using namespace bandit;
using namespace snowhouse;

go_bandit([]() {

  size_t sequence;
  std::vector<SequencedIteratorPair> unchanged_blocks;
  std::vector<SequencedNewData> new_blocks;
  std::string location("/blog");

  std::stringstream log;

  cdnalizer::Config cfg{{{"/images", "http://cdn.supa.ws/imgs"}}};

  RangeEvent<Iterator> unchanged = [&](Iterator start, Iterator end) {
    unchanged_blocks.push_back(SequencedIteratorPair{sequence++, start, end});
    std::string out(start, end);
    log << "Unchanged: " << out << '\n';
    return end;
  };

  DataEvent newData = [&](std::string data) {
    new_blocks.push_back(SequencedNewData(sequence++, data));
    log << "New data: " << data << '\n';
  };

  auto doRewrite = [&](Iterator start, const Iterator &end, const Config &cfg,
                       bool isCSS) {
    return cdnalizer::rewriteHTML(location, cfg, start, end, unchanged, newData,
                                  isCSS);
  };

  before_each([&]() {
    sequence = 1;
    unchanged_blocks.clear();
    new_blocks.clear();
  });

  after_each([&](){
    //std::cout << "Log: " << log.str() << std::endl;
  });

  // Returns true if nothing is changed after running doRewrite
  auto ensureNoChange = [&](const std::string &data, bool isCSS = false) {
    Iterator end = doRewrite(data.cbegin(), data.cend(), cfg, isCSS);
    AssertThat(end, Is().EqualTo(data.cend()));
    AssertThat(unchanged_blocks, HasLength(1));
    auto block = unchanged_blocks.at(0);
    AssertThat(block.sequence, Equals((size_t)1));
    AssertThat(block.start, Is().EqualTo(data.cbegin()));
    AssertThat(block.end, Is().EqualTo(data.cend()));
  };

  describe("Low level Rewrite HTML", [&]() {

    it("1. Returns unchanged when there are no tags", [&]() {
      const std::string data{"There are no tags here"};
      ensureNoChange(data);
    });

    it("2. Returns unchanged when we don't find the tags in the library", [&]() {
      const std::string data{
          R"**(This is <some funny="tag">that</some> <we dont="care"/> about)**"};
      ensureNoChange(data);
    });

    it("3. Ignores sligthly wrong paths we don't care about", [&]() {
      const std::string data{
          R"**(<a src="../images/a.gif"><img href="/home/images/bad.link" />some link</a>)**"};
      ensureNoChange(data);
    });

    it("4. Ignores when location breaks our match", [&]() {
      const std::string data{
          R"**(<a href="images/a.gif"><img src="/images/bad.link" />Bad location</a>)**"};
      cfg.addPath("/blog2/images", "http://cdn.supa.ws/blog2/imags");
      Iterator end = doRewrite(data.cbegin(), data.cend(), cfg, false);
      AssertThat(end, Is().EqualTo(data.cend()));
      AssertThat(unchanged_blocks, HasLength(2));

      // Unchanged: "<a href="images/a.gif"><img src="
      auto block = unchanged_blocks.at(0);
      AssertThat(block, Equals(SequencedIteratorPair{1, data.cbegin(),
                                                     data.cbegin() + 33}));

      // New Data: "http://cdn.supa.ws/imgs"
      AssertThat(new_blocks, HasLength(1));
      auto new_block = new_blocks.at(0);
      AssertThat(new_block,
                 Equals(SequencedNewData{2, "http://cdn.supa.ws/imgs"}));

      // Unchanged: '/bad.link" />Bad location</a>'
      block = unchanged_blocks.at(1);
      AssertThat(block, Equals(SequencedIteratorPair{3, data.cbegin() + 40,
                                                     data.cend()}));
    });

    it("5. Works out location correctly", [&]() {
      const std::string data{
          R"**(<a href="images/a.gif"><img src="/images/bad.link" />Bad location</a>)**"};

      // location is '/blog/', so 'images/a.gif' should be interpreted as
      // '/blog/images/a.gif'
      cfg.addPath("/blog/images", "http://cdn.supa.ws/blog/imags");
      Iterator end = doRewrite(data.cbegin(), data.cend(), cfg, false);
      AssertThat(end, Is().EqualTo(data.cend()));
      AssertThat(unchanged_blocks, HasLength(3));

      // Unchanged: "<a href="
      auto block = unchanged_blocks.at(0);
      AssertThat(block, Equals(SequencedIteratorPair{1, data.cbegin(),
                                                     data.cbegin() + 9}));

      // New Data: "http://cdn.supa.ws/blog/imags"
      AssertThat(new_blocks, HasLength(2));
      SequencedNewData new_block = new_blocks.at(0);
      AssertThat(new_block,
                 Equals(SequencedNewData{2, "http://cdn.supa.ws/blog/imags"}));

      // Unchanged: '/a.gif"><img src='
      block = unchanged_blocks.at(1);
      AssertThat(block, Equals(SequencedIteratorPair{3, data.cbegin() + 15,
                                                     data.cbegin() + 33}));

      // New Data: "http://cdn.supa.ws/imgs"
      new_block = new_blocks.at(1);
      SequencedNewData expected =
          SequencedNewData{4, "http://cdn.supa.ws/imgs"};
      AssertThat(new_block, Equals(expected));

      // Unchanged: '/bad.link" />Bad location</a>'
      block = unchanged_blocks.at(2);
      AssertThat(block, Equals(SequencedIteratorPair{5, data.cbegin() + 40,
                                                     data.cend()}));
    });

    it("6. Works out location with an extra slash correctly", [&]() {
      const std::string data{
          R"**(<a href="images/a.gif"><img src="/images/bad.link" />Bad location</a>)**"};

      location = "/blog/";

      // location is '/blog/', so 'images/a.gif' should be interpreted as
      // '/blog/images/a.gif'
      cfg.addPath("/blog/images", "http://cdn.supa.ws/blog/imags");
      Iterator end = doRewrite(data.cbegin(), data.cend(), cfg, false);
      AssertThat(end, Is().EqualTo(data.cend()));
      AssertThat(unchanged_blocks, HasLength(3));

      // Unchanged: "<a href="
      auto block = unchanged_blocks.at(0);
      AssertThat(block, Equals(SequencedIteratorPair{1, data.cbegin(),
                                                     data.cbegin() + 9}));

      // New Data: "http://cdn.supa.ws/blog/imags"
      AssertThat(new_blocks, HasLength(2));
      SequencedNewData new_block = new_blocks.at(0);
      AssertThat(new_block,
                 Equals(SequencedNewData{2, "http://cdn.supa.ws/blog/imags"}));

      // Unchanged: '/a.gif"><img src='
      block = unchanged_blocks.at(1);
      AssertThat(block, Equals(SequencedIteratorPair{3, data.cbegin() + 15,
                                                     data.cbegin() + 33}));

      // New Data: "http://cdn.supa.ws/imgs"
      new_block = new_blocks.at(1);
      SequencedNewData expected =
          SequencedNewData{4, "http://cdn.supa.ws/imgs"};
      AssertThat(new_block, Equals(expected));

      // Unchanged: '/bad.link" />Bad location</a>'
      block = unchanged_blocks.at(2);
      AssertThat(block, Equals(SequencedIteratorPair{5, data.cbegin() + 40,
                                                     data.cend()}));
    });
  });

  it("7. Handles multiple attribute changes, and with all types of quotes", [&]() {
    const std::string data{
        R"(<a href="images/a.gif" other_attrib=/blog/images/b.gif singles='images/c.gif'><img src="/images/bad.link" />Bad location</a>)"};
    /*     0         1         2         3         4         5         6         7         8         9         0         1         2         3 */

    location = "/blog/";

    // location is '/blog/', so 'images/a.gif' should be interpreted as
    // '/blog/images/a.gif'
    cfg.addPath("/blog/images", "http://cdn.supa.ws/blog/imags");
    Iterator end = doRewrite(data.cbegin(), data.cend(), cfg, false);
    AssertThat(end, Is().EqualTo(data.cend()));
    AssertThat(unchanged_blocks, HasLength(5));

    size_t sequence = 1;
    size_t unchangedIndex = 0;
    size_t newBlockIndex = 0;

    // Unchanged: "<a href="
    AssertThat(unchanged_blocks.at(unchangedIndex++),
               Equals(SequencedIteratorPair{sequence++, data.cbegin(),
                                            data.cbegin() + 9}));

    // New Data: The inside of the tag up until the last path bit of the last
    // attribute
    AssertThat(new_blocks, HasLength(4));
    AssertThat(new_blocks.at(newBlockIndex++),
               Equals(SequencedNewData{sequence++,
                                       R"(http://cdn.supa.ws/blog/imags)"}));

    // Unchanged: 'a.gif" other_attrib='
    AssertThat(unchanged_blocks.at(unchangedIndex++),
               Equals(SequencedIteratorPair{sequence++, data.cbegin() + 15,
                                            data.cbegin() + 36}));

    // New Data:
    AssertThat(new_blocks.at(newBlockIndex++),
               Equals(SequencedNewData{sequence++,
                                       R"(http://cdn.supa.ws/blog/imags)"}));

    // Unchanged: 'b.gif singles='
    AssertThat(unchanged_blocks.at(unchangedIndex++),
               Equals(SequencedIteratorPair{sequence++, data.cbegin() + 48,
                                            data.cbegin() + 64}));

    // New Data: "http://cdn.supa.ws/blog/imags"
    AssertThat(
        new_blocks.at(newBlockIndex++),
        Equals(SequencedNewData{sequence++, "http://cdn.supa.ws/blog/imags"}));

    // Unchanged: '/c.gif"><img src='
    AssertThat(unchanged_blocks.at(unchangedIndex++),
               Equals(SequencedIteratorPair{sequence++, data.cbegin() + 70,
                                            data.cbegin() + 88}));

    // New Data: "http://cdn.supa.ws/imgs"
    AssertThat(new_blocks.at(newBlockIndex++),
               Equals(SequencedNewData{sequence++, "http://cdn.supa.ws/imgs"}));

    // Unchanged: '/bad.link" />Bad location</a>'
    AssertThat(unchanged_blocks.at(unchangedIndex++),
               Equals(SequencedIteratorPair{sequence++, data.cbegin() + 95,
                                            data.cend()}));
  });

  it("8. Handles boolean tags", [&]() {
    const std::string data(
        R"(junky bits <A boolean href="/images/d.gif" check_something_else>click here</a>)");
    Iterator end = doRewrite(data.cbegin(), data.cend(), cfg, false);
    AssertThat(end, Is().EqualTo(data.cend()));

    AssertThat(unchanged_blocks, HasLength(2));

    // Unchanged: "junky bits <A boolean href=""
    auto block = unchanged_blocks.at(0);
    AssertThat(block, Equals(SequencedIteratorPair{1, data.cbegin(),
                                                   data.cbegin() + 28}));
    // New Data: "http://cdn.supa.ws/imgs"
    auto new_block = new_blocks.at(0);
    SequencedNewData expected = SequencedNewData{2, "http://cdn.supa.ws/imgs"};
    AssertThat(new_block, Equals(expected));

    // Unchanged: --d.gif" check_something_else>click here</a>--
    block = unchanged_blocks.at(1);
    AssertThat(block, Equals(SequencedIteratorPair{3, data.cbegin() + 35,
                                                   data.cbegin() + 35 + 43}));



  });

  it("9. rewrites inline style tags", [&]() {
    const std::string data(
        R"--(junky bits <A boolean style="background-image: url('/images/happy.jpg'); filter: url('/images/filter.css');" check_something_else>click here</a>)--");
    Iterator end = doRewrite(data.cbegin(), data.cend(), cfg, false);
    AssertThat(end, Is().EqualTo(data.cend()));

    size_t sequence = 1;
    size_t unchangedIndex = 0;
    size_t newBlockIndex = 0;

    AssertThat(unchanged_blocks, HasLength(3));

    // Unchanged: "junky bits <A boolean style="background-image: url('"
    AssertThat(unchanged_blocks.at(unchangedIndex++),
               Equals(SequencedIteratorPair{sequence++, data.cbegin(),
                                            data.cbegin() + 52}));

    // New Data: "http://cdn.supa.ws/imgs"
    AssertThat(new_blocks.at(newBlockIndex++),
               Equals(SequencedNewData{sequence++, "http://cdn.supa.ws/imgs"}));

    // Unchanged: --happy.jpg'); filter: url('--
    AssertThat(unchanged_blocks.at(unchangedIndex++),
               Equals(SequencedIteratorPair{sequence++, data.cbegin() + 59,
                                            data.cbegin() + 59 + 27}));

    // New Data: "http://cdn.supa.ws/imgs"
    AssertThat(new_blocks.at(newBlockIndex++),
               Equals(SequencedNewData{sequence++, "http://cdn.supa.ws/imgs"}));


    // Unchanged: --happy.jpg'); filter: url('--
    AssertThat(unchanged_blocks.at(unchangedIndex++),
               Equals(SequencedIteratorPair{sequence++, data.cbegin() + 93,
                                            data.cbegin() + 144}));


  });

  it("10. doesn't rewrite php files", [&]() {
    const std::string data(
        R"--(junky bits 
              <img src="/images/bad.php" /> Shouldn't be rewritten because it ends .php
              <img src="/images/bad2.php?x=2" /> Shouldn't be rewritten because it calls bad2.php passing get parameters
              <img src="/images/good.jpg" />  Should be rewritten because it's in /images/folder
              <img src="/images/good.jpg?x=.php&y=1" />  Should be rewritten because it's in /images/folder - it should ignore the .php
              <img src="/images/good.jpg?x=.php" />  Special bug - in favor of speed, we assume it's a .php because it ends in that. It's unlikely that someone's going to pass get parameters to a real jpg
              <A boolean style="background-image: url('/images/happy.jpg'); filter: url('/images/filter.css');" check_something_else>both urls should be rewritten</a>
              <A boolean style="background-image: url('/images/happy.php'); filter: url('/images/filter.css.php');" check_something_else>Both should not be rewritten</a>)--");
    const std::string expected(
        R"--(junky bits 
              <img src="/images/bad.php" /> Shouldn't be rewritten because it ends .php
              <img src="/images/bad2.php?x=2" /> Shouldn't be rewritten because it calls bad2.php passing get parameters
              <img src="http://cdn.supa.ws/imgs/good.jpg" />  Should be rewritten because it's in /images/folder
              <img src="http://cdn.supa.ws/imgs/good.jpg?x=.php&y=1" />  Should be rewritten because it's in /images/folder - it should ignore the .php
              <img src="/images/good.jpg?x=.php" />  Special bug - in favor of speed, we assume it's a .php because it ends in that. It's unlikely that someone's going to pass get parameters to a real jpg
              <A boolean style="background-image: url('http://cdn.supa.ws/imgs/happy.jpg'); filter: url('http://cdn.supa.ws/imgs/filter.css');" check_something_else>both urls should be rewritten</a>
              <A boolean style="background-image: url('/images/happy.php'); filter: url('/images/filter.css.php');" check_something_else>Both should not be rewritten</a>)--");
    std::string whatWeGot;
    RangeEvent<Iterator> unchanged = [&](auto start, auto end) {
      std::copy(start, end, std::back_inserter(whatWeGot));
      return end;
    };
    DataEvent newData = [&](std::string data) { whatWeGot.append(data); };
    auto newEnd = cdnalizer::rewriteHTML(location, cfg, data.begin(),
                                         data.end(), unchanged, newData, false);
    AssertThat(whatWeGot, Equals(expected));
    });
});

int main(int argc, char **argv) { return bandit::run(argc, argv); }
