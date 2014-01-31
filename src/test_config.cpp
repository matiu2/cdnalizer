#include "Config.hpp"

using namespace cdnalizer;

bool operator ==(const Config::CDNRefPair& a, const Config::CDNPair& b) {
    return (a.first == b.first) && (a.second == b.second);
}

#include <bandit/bandit.h>

using namespace bandit;

bool operator ==(const Config::CDNPair& a, const Config::CDNRefPair& b) {
    return (a.first == b.first) && (a.second == b.second);
}

/// Print CDNPairs
std::ostream& operator <<(std::ostream& s, const Config::CDNRefPair& p) {
    s << "< " << p.first << ", " << p.second << " >";
    return s;
}

/// Print CDNPairs
std::ostream& operator <<(std::ostream& s, const Config::CDNPair& p) {
    s << "< " << p.first << ", " << p.second << " >";
    return s;
}

go_bandit([&](){

    Container map {{
        {"/images", "http://cdn.supa.ws/imgs"},
        {"/images2", "http://cdn.supa.ws/imgs2"},
        {"/aaa", "http://cdn.supa.ws/aaa"},
        {"/aab", "http://cdn.supa.ws/aab"},
        {"/aac", "http://cdn.supa.ws/aac"}
    }};

    describe("Config", [&](){
        it("1. finds path - cdn_url pairs", [&] {
            Config cfg{Container{map}};
            // Find aaa
            Config::CDNRefPair aaa = cfg.findCDNUrl("/aaa/x.gif");
            Config::CDNPair expected{"/aaa", "http://cdn.supa.ws/aaa"};
            AssertThat(aaa, Equals(expected));
            // Find aab
            Config::CDNRefPair aab = cfg.findCDNUrl("/aab/x.gif");
            expected = {"/aab", "http://cdn.supa.ws/aab"};
            AssertThat(aab, Equals(expected));
            // Find aac
            Config::CDNRefPair aac = cfg.findCDNUrl("/aac/x.gif");
            expected = {"/aac", "http://cdn.supa.ws/aac"};
            AssertThat(aac, Equals(expected));
        });
        it("2. add path works", [&] {
            Config cfg{Container{map}};
            // Shouldn't be there at the beginning
            Config::CDNRefPair aad = cfg.findCDNUrl("/aad/x.gif");
            Config::CDNPair expected{"/aad", "http://cdn.supa.ws/aad"};
            AssertThat(aad, !Equals(expected));
            // Add path should work
            cfg.addPath("/aad", "http://cdn.supa.ws/aad");
            Config::CDNRefPair aad2 = cfg.findCDNUrl("/aad/x.gif");
            AssertThat(aad2, Equals(expected));
        });
    });
});

int main(int argc, char** argv) {
    return bandit::run(argc, argv);
}
