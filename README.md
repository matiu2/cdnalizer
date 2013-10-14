# CDNalizer

This project provides an nginx and apache plugin that re-writes your outgoing HTML. It changes your image src tags and whatnot to point to your cloudfiles directly.

## Archetecture

### Components

 * Config Reader
   * Apache Config Reader
   * nginx config reader
 * Rewriter
 * Apache wrapper
 * Nginx wrapper

# TODO

  * TestRewriter
 * Rewriter
 * ...

## Useful developer links

 * http://wiki.apache.org/nutch/WritingPluginExample
 * http://httpd.apache.org/docs/2.2/filter.html
 * http://httpd.apache.org/docs/2.2/filter.html
 * http://httpd.apache.org/docs/2.2/mod/mod_filter.html
 * http://httpd.apache.org/docs/trunk/developer/output-filters.html

# Future

 * Threading
 * Caching by url + quick checksum
 * Have it handle uploads and spool them straight up to cloud files
