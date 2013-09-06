#include "Rewriter.hpp"
#include "Config.hpp"

#include <string>
#include <iostream>
#include <map>
#include <functional>

struct Test;

/// The default configuration for our tests
cdnalizer::Config cfg{
    { 
        {"a", "href"},
        {"img", "src"}
    },
    {
        {"/a/", "http://cdn.supa.ws/container_a/"},
        {"/b_", "http://cdn.supa.ws/container_x/special/b_"},
        {"/here/", "http://cdn.supa.ws/implicit/"},
        {"http://old.cdn/", "http://new.cdn/"}
    }
};


/// Runs a test and outputs the result
struct Test {
    std::string name,
                in,
                expected;
    bool operator() () {
        using std::cout;
        using std::endl;
        cout << name << " - ";
        std::string out = rewriteHTML("/here", cfg, in);
        bool isGood = out == expected;
        if (isGood)
            cout << "PASSED";
        else
            cout << "FAILED";
        cout << std::endl << "INPUT:    " << in
             << std::endl << "OUTPUT:   " << out
             << std::endl << "EXPECTED: " << expected
             << std::endl << std::endl;
        return isGood;
    }
};

/// List of all tests that we can run by name
std::map<std::string, Test> tests {
    {"badTag", {
        {"badTag: tag we don't care about"},
        {R"(<input type="text">)"},
        {R"(<input type="text">)"}}
    },
    {"badAttribVal", {
        {"badAttribVal: attrib we don't care about"},
        {R"(<a href="/c/d/e.html">not me</a>)"},
        {R"(<a href="/c/d/e.html">not me</a>)"}}
    },
    {"missingAttrib", {
        {"badAttribVal: attrib we don't care about"},
        {R"(<a not_href="/a/good.html">not me</a>)"},
        {R"(<a not_href="/a/good.html">not me</a>)"}}
    },
    {"changeA", {
        {"changeA: attrib we don't care about"},
        {R"(<a href="/a/good.html">not me</a>)"},
        {R"(<a href="http://cdn.supa.ws/container_a/good.html">not me</a>)"}}
    },
};

int fullTest() {
    using namespace cdnalizer;
    std::string html{
        R"(<input type="text">
        <a href="c/d/e.html">not me</a>
        <img src="/nowhere.png" />
        <img src="/a/somewhere.png" />
        <img src="/b_somewhere.png" />
        <a href="/not_a/a/download.pdf">don't get me</a>
        <a href="/a/download.pdf">do get me</a>
        <a href="implicit.pdf">get me too</a>
        <a href="http://old.cdn/old.pdf">get me too</a>
        )"
    };
    rewriteHTML("/here", cfg, html);
    std::string expected{
        R"(<input type="text">
        <a href="c/d/e.html">not me</a>
        <img src="/nowhere.png" />
        <img src="http://cdn.supa.ws/container_a/somewhere.png" />
        <img src="http://cdn.supa.ws/container_x/special/b_somewhere.png" />
        <a href="/not_a/a/download.pdf">don't get me</a>
        <a href="http://cdn.supa.ws/container_a/download.pdf">do get me</a>
        <a href="http://cdn.supa.ws/implicit/implicit.pdf">get me too</a>
        <a href="http://new.cdn/old.pdf">get me too</a>
        )"
    };
    if (html != expected)
        return 1;
    return 0;
}

void printUsage(const char* progName) {
    std::cout << "Usage: " << progName << " TEST_NAME"
              << std::endl << "Where TEST_NAME is one of: "
              << std::endl << std::endl;
    for (auto test : tests) {
        std::cout << test.first << std::endl;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::vector<Test> testsToRun;
    
    // Run all tests
    if (std::string(argv[1]) == "ALL")  {
        for (auto test : tests)
            testsToRun.push_back(test.second);
    } else {
        // Run selected Tests
        for (int i=1; i<argc; ++i) {
            auto found = tests.find(argv[i]);
            if (found == tests.end()) {
                std::cout << "Couldn't find test named '" << argv[i] << "'" << std::endl;
                printUsage(argv[0]);
                return 2;
            } else {
                testsToRun.push_back(found->second);
            }
        }
    }

    // Run the tests we've chosen
    int result = 0;
    for (auto test : testsToRun) {
        bool res2 = test();
        if (!res2)
            result = 3;
    }
    return result;
}
