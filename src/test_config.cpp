#include "Config.hpp"

using namespace cdnalizer;

/// Config::CDNPair is a reference to existing strings; we want this type to actually hold the strings
using Pair = std::pair<std::string, std::string>;

bool operator ==(const Config::CDNPair& a, const Pair& b) {
    return (a.first == b.first) && (a.second == b.second);
}

#include <bandit/bandit.h>

using namespace bandit;

bool operator ==(const Pair& a, const Config::CDNPair& b) {
    return (a.first == b.first) && (a.second == b.second);
}

/// Print CDNPairs
std::ostream& operator <<(std::ostream& s, const Config::CDNPair& p) {
    s << "< " << p.first << ", " << p.second << " >";
    return s;
}

/// Print CDNPairs
std::ostream& operator <<(std::ostream& s, const Pair& p) {
    s << "< " << p.first << ", " << p.second << " >";
    return s;
}

go_bandit([&](){

    Config::Container map {{
        {"/images", "http://cdn.supa.ws/imgs"},
        {"/images2", "http://cdn.supa.ws/imgs2"},
        {"/aaa", "http://cdn.supa.ws/aaa"},
        {"/aab", "http://cdn.supa.ws/aab"},
        {"/aac", "http://cdn.supa.ws/aac"}
    }};

    describe("Config", [&](){
        it("1. finds path - cdn_url pairs", [&] {
            Config cfg{Config::Container{map}};
            // Find aaa
            Config::CDNPair aaa = cfg.findCDNUrl("/aaa/x.gif");
            Pair expected{"/aaa", "http://cdn.supa.ws/aaa"};
            AssertThat(aaa, Equals(expected));
            // Find aab
            Config::CDNPair aab = cfg.findCDNUrl("/aab/x.gif");
            expected = {"/aab", "http://cdn.supa.ws/aab"};
            AssertThat(aab, Equals(expected));
            // Find aac
            Config::CDNPair aac = cfg.findCDNUrl("/aac/x.gif");
            expected = {"/aac", "http://cdn.supa.ws/aac"};
            AssertThat(aac, Equals(expected));
        });
        it("2. add path works", [&] {
            Config cfg{Config::Container{map}};
            // Shouldn't be there at the beginning
            Config::CDNPair aad = cfg.findCDNUrl("/aad/x.gif");
            Pair expected{"/aad", "http://cdn.supa.ws/aad"};
            AssertThat(aad, !Equals(expected));
            // Add path should work
            cfg.addPath("/aad", "http://cdn.supa.ws/aad");
            Config::CDNPair aad2 = cfg.findCDNUrl("/aad/x.gif");
            AssertThat(aad2, Equals(expected));
        });
    });
});

int main(int argc, char** argv) {
    return bandit::run(argc, argv);
}
