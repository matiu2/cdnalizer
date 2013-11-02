# Developer Documentation

----

# Features

## 1. Rewrite HTML as it passes through, changing /images/a.gif to http://yourcdn.com/images/a.gif

## 2. Work on stdin and stdout

## 3. Work as an apache filter

## 4. Work as an nginx filter

## 5. Have a generic configuration that can work with all of the above

-----

# New Bugs

## 1. Generates too much new output.

It doesn't need to generate extra data at the end of a link. eg. if we find "/images/a.gif" we only need generate "http://cdn.yours.com" then the "/images/a.gif" can be straight pushed out unchanged.

## 2. Long tags

Needs to handle tags that go over multiple buffers. What if you have a 20k tag ?

----

# Bugs that need testing

## 3. Handle bad syntax

At the moment an end tag is just '>'. We should also take '<' to mean that the previous tag ends here.

-----

# Plans
