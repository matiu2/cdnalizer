#pragma once
/** Configuration Holder
 *
 * # Config attributes
 *  * Tag Attrib:
 *    * img src
 *    * a href
 *  * Values to change:
 *    * /my/images/ ==> https://cdn.supa.ws/my_images/
 *    * something   ==> http://cdn.supa.ws/something/somewhere/prefix_
 */

#include <vector>
#include <string>
#include <map>

namespace cdnalizer {

struct Config {
    using Map = std::map<std::string, std::string>;
    /// Pairs of 'tag to change' + 'attribute to change'. eg. {{"img","src"}, {"a", "href"}}
    Map tag_attrib = {
        {"a", "href"},
        {"img", "src"}
    };
    Map paths;
};

}
