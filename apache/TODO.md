# Plan

We've used `apxs -g -n cdnalizer` to generate the 'cndalizer' directory in here, which is an apache module source code directory.

Use `sudo apxs -c -i mod_cdnalizer.c` to install it.

# Todo:

## Make it work with a default config

First step is to just see the monitoring in action. We'll need to make a extern C wrapper and a static library for the module to call.

## Add Configuration options

Next we'll need to be able to load a config from the apache variables.
