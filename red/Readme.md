# Do very little with the R3D SDK

Download the SDK from red.com, and you may find that it's...  pretty spartan.  It desn't even have Makefiles.  So, here's a slightly more complete example.  It doesn't do very much.  But it has a CMake file that you cn use to get up and running to make sure the SDK is working before you start hacking on your Killer App.

This example depends on OpenImageIO, because it creates an OIIO ImageBuf with the decoded frame, so it can save it in a real format, rather than just dumping some raw data to disk like a savage.

It also includes a few small convenience functions that I found useful when I first started poking at the SDK for debugging and confidence building.  Most likely, none of them really belong in production code, but that may be handy.

Everything I've written here is hereby under the BSD license.  (license file at the root of this repo.)  This should mean you can feel free to copy/change/use it as you like.


