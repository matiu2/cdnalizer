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

# Future

 * Have it handle uploads and spool them straight up to cloud files

