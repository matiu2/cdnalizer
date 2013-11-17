#include "iterator.hpp"

#include <bandit/bandit.h>

#include <string>
#include <sstream>
#include <unordered_map>

#define TEST(name) = {"name", std::bind(this, &Tester::name)}

using namespace bandit;
using cdnalizer::stream::Iterator;

go_bandit([](){
    describe("Empty Iterator", [&](){
        std::stringstream data;

        before_each([&]() {
            data.str("0123456789");
        });

        it("Should be creatable taking a stream", [&](){
            Iterator a(data);
        });

        it("Should return the character it points to", [&](){
            Iterator a(data);
            AssertThat(*a, Is().EqualTo('0'));
        });

    });
});

namespace cdnalizer {
namespace stream {

struct Tester {
    std::stringstream data{"0123456789"};
    Iterator a{data};
    /// Should be able to read a char
    void canRead() {
        Iterator a(data);
        assert(*a == '0');
    }
    void basicInc() {
        Iterator a(data);
        assert(*a++ == '0');
        assert(*a == '1');
    }
    void  run() {
        canRead();
        basicInc();
    }
};

}

}

int main(int argc, char** argv) {
    bandit::run(argc, argv);
}
