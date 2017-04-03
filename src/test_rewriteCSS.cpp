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

  describe("Low level Rewrite HTML", [&]() {

    it("1. Returns unchanged when there are no paths", [&]() {
      const std::string data{"There are no paths here"};
      std::string unchanged;
      std::string newData;
      RangeEvent<iterator> unchangedEvent =
          [&](const iterator &a,
              const iterator &b) -> iterator {
        std::copy(a, b, std::back_inserter(unchanged));
        return b;
      };
      auto newDataEvent = [&newData](std::string aNewData) {
        newData = aNewData;
      };
      return cdnalizer::rewriteHTML<iterator>(
          server, location, cfg, data.cbegin(), data.cend(), unchangedEvent,
          newDataEvent, true);
      AssertThat(unchanged, Is().EqualTo(data));
      AssertThat(newData, Is().EqualTo(""));
    });

  });

});

int main(int argc, char **argv) { return bandit::run(argc, argv); }
