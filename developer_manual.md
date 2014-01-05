# Devoleper manual

# Dependencies for compiling

 * An apache with debug symbols.
 * cgdb - nice debugger. Optional of course.
 * bandit - testing suite - `git submodule update` to get it.
 * cmake - compile configuration program
 * ccmake - cmake gui (optional)
 * ctest - test running stuff

# Instructions for compiling

    mkdir build
    cd build
    cmake ..
    ccmake .. # Edit vars as needed
    make -j4
    ctest

# Instructions for testing in apache.

    cd build/dev-tools
    ./debug-apache.sh
    #(r for run) 
    curl localhost:8000 # In another terminal to trigger the page generation

# Archetecture

## src/

The src base directory contains the heart of the program.

 * Config.hpp - Holds the configuration - you must supply your own front end to read and write configurations; this just holds it.
 * Rewriter.hpp - Public interface to use the main algorithm
 * Rewriter_impl.hpp - Implementation of the main algorithm. Needs a ForwardIterator. Does the search and replace, trying to do as little data copying as possible
 * pair.hpp - A pair of iterators (think const char*), like a string but refers to existing data rather than copying it.
 * utils.hpp - Little functions that can be useful throughout the program

## src/apache

Source for the mod_cdnalizer apache Module.

## dev-tools

At the moment tools to help debug the apache module



The algorithm is in src/Rewriter_impl.hpp ..
