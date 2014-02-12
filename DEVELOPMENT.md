# Dev docs

## TODO

### Standalone program

 * Have it read a config file

### Apache filter

 * Have it detect CDN_URL config recursion loops
     * Have it generate error logs for the recursion
 * Have it support DEL_CDN_URL config directive
 * Have it work on centos

### NGINX filter

 * Start writing it
 * Fill in rest of todo list

# Archetecture

## Components

 * /src -- contains all source code
     * Config.hpp -- Holds a configuration object
     * Rewriter.hpp and Rewriter_impl.hpp -- The actual HTML re-writing algorithnm
     * pair.hpp -- internal class to help read in buffers with less copying; pair of iterators into a buffer
     * utils.hpp -- internal utility funcs and classes

 * /src/stream/ -- Just used for testing and standalone, acts on a stream given a forward iterator and an output iterator 
 * /src/standalone/ -- Command line read and write files
 * /src/apache/ -- Everything apache
   * config.hpp -- Handles apache configuration callbacks
   * config.cpp --
   * mod_cdnalizer.hpp -- The main module and callback hook organizer
   * mod_cdnalizer.cpp -- 
   * AbstractBlockIterator.hpp -- A forward iterator, that les you use blocks of buffers as if they were one big buffer
   * iterator.hpp -- Lets you treat pointers to blocks of chars as a pchar (almost)(up to the level of a ForwardIterator to char)
   * filter.hpp -- The Apache output filter coordinator
   * utils.hpp -- bits and pieces to make integration with Apache easier

# Useful developer links

 * http://wiki.apache.org/nutch/WritingPluginExample
 * http://httpd.apache.org/docs/2.2/filter.html
 * http://httpd.apache.org/docs/2.2/filter.html
 * http://httpd.apache.org/docs/2.2/mod/mod_filter.html
 * http://httpd.apache.org/docs/trunk/developer/output-filters.html

# Future

 * Have it handle uploads and spool them straight up to cloud files
