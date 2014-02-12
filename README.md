# CDNalizer

cee-dee-en-a-lies-ur

## What's it good for ?

It lets you move static content (images/css/js) to a CDN, without having to modify your app; just one line of Apache config.

For example if you moved everything in your /images folder to http://my.cdn.wiz/images .. you'd then enter this line in Apache:

CDN_URL /images/ http://my.cdn.wiz/images/

### "But what about mod_rewrite!?"

This is not the same.

With mod_rewrite, your HTML goes out like this:

<img src="/images/x.gif" />

Then your client's browser hits your server again, asking for x.gif; mod_rewrite then tells your browser, "no dude, it's not here! It's at http://my.cdn.wiz/images/x.gif

On the plus side, the browser will remember the 'permanent' redirect for several minutes and won't ask again for that image for ages.

----

With mod_cdnalizer your HTML goes out like this:

<img src="http://my.cdn.wiz/images/x.gif" />

Your browser goes straight to the CDN, and cuts out that intermediary request.

So as you can you can imagine, when you multiply number of static files (image/css/js) by number of users (then minus the repeat hits) .. mod_cdnalizer can save your server a lot of useless hits.

----

### "Why don't I just change my app to rewrite the HTML?"

Go on then, do that. If you can change your app easily and have good processes in place to do so, then mod_cdnalizer is not for you.

One day there was Mister X. He painstakingly changed his app to rewrite all the outgoing urls to point to the CDN. Everything was going well, when suddenly, Mister X accidentally deleted the CDN folder.

"Oh no! my sites are all showing crap with no JS, no CSS, and 'x's where there should be images! Quick quick change the app to point back to the local files!"

Too late for Mister X; it took 3 days to change his app back. If only he would have used mod_cdnalizer, he would have been able to comment out 3 Apache directives in a minute and had all his sites look nice again in 5 minutes.

----

### "What about speed? Won't it slow down all the outgoing HTML?"

I thought that too. I thought, I'll take an MD5 of pages and cache the results, but you know what happened ? it turned out that rewriting the page on the way out is faster than taking an MD5.

As your app is probably written in speedy PHP/Ruby/Python .. and mod_rewrite is painstakingly written is in c++ with speed in mind (eg. trying to reduce memory copying as much as possible) .. I'm sure there are many other areas that you could change to improve your speed before hitting up mod_cdnalizer.

----
