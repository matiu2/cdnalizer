%%{

# Checks to see if a path/url is static/php

machine path;

action done {
    // Return false because the path is not static
    return false;
}

php = ".php";
pl = ".pl";
py = ".py";
extensions = php | pl | py;
path := (any - '?')* extensions %done ('?' >done)? ;

}%%
