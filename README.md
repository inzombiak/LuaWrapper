# LuaWrapper
A wrapper for Lua written in C++11. Work in progress.

As the title says, a mostly bug free Lua wrapper. I've used it in [BeatHeroes](https://github.com/inzombiak/BeatHeroes), [Material 2D](https://github.com/inzombiak/Material2D) and [LuaRooms](https://github.com/inzombiak/LuaRooms)

I wrote this following Jeremy Ong's blog posts on his Lua wrapper, [Selene](https://github.com/jeremyong/Selene). I have no idea why I didn't fork it back then (maybe something to do with the lack of \_ENV usage/usage of underscore functions?), but I'll try to get around to doing that at some point.

On issue the library has is that I fully clean the stack after each call to the wrapper to avoid malformed stacks. While this works, and has no bugs, I think each call should be written in a way that it can clean up after itself. I'll try to address this at some point.

# TODO
* Get rid of CleanStack()
