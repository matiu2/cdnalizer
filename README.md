# CDNalizer

cee-dee-en-a-lies-ur

## What's it good for ?

It helps makes your website faster and reduce load on your webserver.

## How do I use it ?

Copy some static content (images/css/js) up to a push only CDN (like Rackspace Cloud Files), then use CDNalizer to update your app.

The way I use it is:

 1. Use [LSyncd](https://github.com/axkibe/lsyncd lsyncd) to watch a directory
 2. Have LSyncd use [turbolift] to sync all file changes made there up to the CDN
 3. In your .htaccess file add: `CDN_URL /uploads/ http://mycdn.supa.ws/uploads/`

## How to turn it off ?

Comment out that `CDN_URL`, that'll pretty much instantly return things to normal.

## Where can I get it ?

Download a package from here: http://cdnalizer.supa.ws/

## How does it work ?

It rewrites your HTML as it goes out, so your app generates: `<img src="/uploads/fun.png" />` and CDNalizer re-writes it on the fly to something like `<img src="http://mycdn.supa.ws/uploads/fun.png" />`

### "But what about mod_rewrite!?"

This is not the same.

With mod_rewrite, your HTML goes out like this:

    <img src="/images/x.gif" />

Then your client's browser hits your server again, asking for x.gif; mod_rewrite then tells your browser, "no dude, it's not here! It's at http://my.cdn.wiz/images/x.gif

With mod_cdnalizer your HTML goes out like this:

    <img src="http://my.cdn.wiz/images/x.gif" />

Your browser goes straight to the CDN, and cuts out that intermediary request.

So as you can you can imagine, when you multiply number of static files (image/css/js) by number of users (then minus the repeat hits) .. mod_cdnalizer can save your server a lot of useless hits and extra load.

----

### "Why don't I just change my app to rewrite the HTML?"

Go on then, do that. If you can change your app easily and have good processes in place to do so, then mod_cdnalizer is not for you.

One day there was Mister X. He painstakingly changed his app to rewrite all the outgoing urls to point to the CDN. Everything was going well, when suddenly, Mister X accidentally deleted the CDN folder.

"Oh no! my sites are all showing crap with no JS, no CSS, and 'x's where there should be images! Quick quick change the app to point back to the local files!"

Too late for Mister X; it took 3 days to change his app back. If only he would have used mod_cdnalizer, he would have been able to comment out 3 Apache directives in a minute and had all his sites look nice again in 5 minutes.

----

### "What about speed? Won't it slow down all the outgoing HTML?"

I thought that too. I thought, I'll take an MD5 of pages and cache the results, but you know what happened ? it turned out that rewriting the page on the way out is faster than taking an MD5 in all my test cases.

When I wrote CDNalizer, I was really pedantic on keeping things fast, I go out of my way to not copy strings around, to re-use parts of the URL that don't need changing, etc.
As your app is probably written in speedy PHP/Ruby/Python .. and mod_rewrite is painstakingly written is in c++ with speed in mind (eg. trying to reduce memory copying as much as possible) .. I'm sure there are many other areas that you could change to improve your speed before hitting up mod_cdnalizer.

----

### How do I get this magical elixir ?

On Ubuntu 13.10 and 14.04:

apt-get update
apt-get install apache2 -y
wget https://raw.githubusercontent.com/matiu2/cdnalizer/master/binaries/ubuntu-13.10/cdnalizer-0.1-Linux.deb
dpkg -I cdnalizer

On other platforms:

Stand by, I'm working on a build machine with CDash that should do the trick.
