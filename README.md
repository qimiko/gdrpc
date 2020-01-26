# Geometry Dash 2.1 Rich Presence

## A semi recreation and port of my 1.9 rich presence

### Currently features

* Official levels
  * Includes The Challenge
* Saved levels
* Editor
  * Designed to not display name
* Menu state
* Autoloads

**Note**: This thing will probably crash if the servers go down, considering that there's many reasons for the servers to go down and many responses it will give.. Fixes will be made as time goes on...

### Using

#### Manually

1. inject `gdrpc.dll` with DLL injector

#### Automatically

1. copy `libcurl.dll` and `gdrpc.dll` to the Geometry Dash directory
2. start GD

### Compiling

1. import files into Visual Studio and make sure it can build a dll
2. edit `DiscordSecret.cpp` and make it yours
3. include Discord files, make sure `discord-rpc.lib` is in main folder
4. build dll
5. add the assets in the `assets` folder to discord

#### Autoloader

The autoloader can be found at [this repo](https://github.com/zmxhawrhbg/curl). Compile it like you would curl (make sure it is as DLL).  
No libs were added to the build.

### Credits

* Absolute for level pointer from mega hack
* nekit for the level location thingy
* cos8o for being cos8o
* saya for motivation
* silentzer for being smart

### TODOs

* Recoding main loop
* Get info without requests in case of server failure
* 🦀
