# Developer Documentation

----

# Features

## 1. Rewrite HTML as it passes through, changing /images/a.gif to http://yourcdn.com/images/a.gif

## 2. Work on stdin and stdout

## 3. Work as an apache filter

## 4. Work as an nginx filter

## 5. Have a generic configuration that can work with all of the above

## 6. Investigate a forward iterator version. See if it's faster/better.

A forward iterator version would possibly need to do a lot more bucket splits than the random iterator version. Something like this:

    some text <a some="attribute" href="/images/a.gif" />
    ^                                 ^ ^
    \start          Found our iterator/ \copy now

When it hit the end of an Apache bucket, it'd just do a push out, which would also give the advantage of less complication between the two layers
(buckets and the algorithm).

The forward iterator would need to maintain a state:

 * out of tag
 * in tag - start (getting tag name)
 * in tag - in space (got tag name, looking for first attribute)
 * in tag - in attrib_name (looking for '=' or '<' or '>')
 * in tag - got equals (looking for '"' or '<' or '>')
 * in tag - in attrib_value (looking for closing '"' or '<' or '>'

Could be done with ragel.

### Advantages

Easier pointer management, no buffering required for stdin, no extra layer between Apache buckets and the algorithm.

-----

# New Bugs

## 2. Long tags

Needs to handle tags that go over multiple buffers. What if you have a 20k tag ?

### Glossary:

 * buffer - series of data (char* + length)
 * bucket - buffer for apache filter, reference counted and a bit magic, but can be turned into a buffer
 * brigade - a linked list of buckets
 * istream - c++ istream

### Situation

There's a tension between how the input is read:

 * The apache filter uses bucket brigades
 * The stdein driver uses istream (with no random access)
 * I don't know how nginx filters work yet

### Plan

Templatize the Rewrite filter, so it can take any random access iterator

Write a wrapper for apache, where an iterator has the bucket addess, plus the offset into it.

For the stdin wrapper buffer it through a temporary file, to allow going back to the start of the current tag

As the Apache filter has a series of buckets (a linked list of buffers), but the Rewriter uses random access iterators (straight pointers),
we will templatize the algorithm to use any random access iterator, then write wrappers for the buckets, and istreams.

----

# Bugs that need testing

## 3. Handle bad syntax

At the moment an end tag is just '>'. We should also take '<' to mean that the previous tag ends here.

## 1. Generates too much new output.

It doesn't need to generate extra data at the end of a link. eg. if we find "/images/a.gif" we only need generate "http://cdn.yours.com" then the "/images/a.gif" can be straight pushed out unchanged.

-----

# Plans
