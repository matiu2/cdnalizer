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

        it("Should be incrementable", [&](){
            Iterator a(data);
            for (char x='0'; x<='9'; ++x) 
                AssertThat(*a++, Is().EqualTo(x));
        });

        it("Should be pre-incrementable", [&](){
            Iterator a(data);
            for (char x='1'; x<='9'; ++x) 
                AssertThat(*(++a), Is().EqualTo(x));
        });

        it("Should be copyable", [&](){
            Iterator a(data);
            Iterator b(a);
            AssertThat(*a, Is().EqualTo(*b).EqualTo('0'));
        });
    });

    describe("Iterator Pair", [&](){
        std::stringstream data;

        before_each([&]() {
            data.str("0123456789");
        });

        it("Should be able to close the gap", [&](){
            Iterator a(data);
            Iterator b(a);
            AssertThat(*a, Is().EqualTo(*b).EqualTo('0'));


        
        });

    });
});

int main(int argc, char** argv) {
    bandit::run(argc, argv);
}
