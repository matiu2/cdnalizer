#include "Config.hpp"

namespace cdnalizer {
    /// Make sure the empty string is an empty string
    const std::string Config::empty = "";

    const Container& get_default_tag_attrib() {
        static Container result;
        if (result.size() == 0) {
            typedef std::pair<std::string, std::string> pair;
            result.insert(pair("a", "href"));
            result.insert(pair("applet", "codebase"));
            result.insert(pair("area", "href"));
            result.insert(pair("audio", "src"));
            result.insert(pair("base", "href"));
            result.insert(pair("bgsound", "src"));
            result.insert(pair("blockquote", "cite"));
            result.insert(pair("embed", "src"));
            result.insert(pair("frame", "src"));
            result.insert(pair("iframe", "src"));
            result.insert(pair("img", "src"));
            result.insert(pair("input", "src"));
            result.insert(pair("link", "href"));
            result.insert(pair("layer", "src"));
            result.insert(pair("object", "usemap"));
            result.insert(pair("q", "url"));
            result.insert(pair("script", "url"));
            result.insert(pair("style", "src"));
        }
        return result;
    }

}
