/**
 * Tests RewriteCSS at a low level.
 * Ensure's events are triggered correctly and that blocks are processed
 *correctly.
 **/
#include "Config.hpp"
#include "Rewriter.hpp"
#include "Rewriter_impl.hpp"

#include <bandit/bandit.h>
#include <ostream>

using namespace cdnalizer;

// Custom Operators

// Acutal tests
using namespace bandit;
using namespace snowhouse;

go_bandit([]() {

  std::string server = "https://supa.ws";
  std::string location("/blog");
  cdnalizer::Config cfg{{{"/images", "http://cdn.supa.ws/imgs"}}};

  using iterator = std::string::const_iterator;

  std::vector<std::string> unchanged, newData;
  RangeEvent<iterator> unchangedEvent = [&](const iterator &a,
                                            const iterator &b) -> iterator {
    std::string tmp;
    std::copy(a, b, std::back_inserter(tmp));
    unchanged.emplace_back(std::move(tmp));
    return b;
  };
  auto newDataEvent = [&newData](std::string aNewData) {
    newData.emplace_back(std::move(aNewData));
  };

  describe("Rewrite CSS", [&]() {

    it("1. Returns unchanged when there are no paths", [&]() {
      const std::string data{"There are no paths here"};
      return cdnalizer::rewriteHTML<iterator>(
          server, location, cfg, data.cbegin(), data.cend(), unchangedEvent,
          newDataEvent, true);
      AssertThat(unchanged, HasLength(1));
      AssertThat(unchanged.at(0), Is().EqualTo(data));
      AssertThat(newData, HasLength(0));
    });

    it("2. Picks up paths with no quotes and no spaces", [&]() {
      const std::string data{"background-image(/images/b.gif)"};
      return cdnalizer::rewriteHTML<iterator>(
          server, location, cfg, data.cbegin(), data.cend(), unchangedEvent,
          newDataEvent, true);
      AssertThat(unchanged, HasLength(2));
      AssertThat(unchanged.at(0), Is().EqualTo("background-image("));
      AssertThat(newData, HasLength(1));
      AssertThat(newData.at(0), Is().EqualTo("https://cdn.supa.ws"));
      AssertThat(unchanged.at(0), Is().EqualTo("/b.gif)"));
    });

    it("3. Picks up paths with no quotes but spaces", [&]() {
      const std::string data{"background-image(   /images/b.gif   )"};
      return cdnalizer::rewriteHTML<iterator>(
          server, location, cfg, data.cbegin(), data.cend(), unchangedEvent,
          newDataEvent, true);
      AssertThat(unchanged, HasLength(2));
      AssertThat(unchanged.at(0), Is().EqualTo("background-image(   "));
      AssertThat(newData, HasLength(1));
      AssertThat(newData.at(0), Is().EqualTo("https://cdn.supa.ws"));
      AssertThat(unchanged.at(0), Is().EqualTo("/b.gif   )"));
    });

  });

});

int main(int argc, char **argv) { return bandit::run(argc, argv); }
