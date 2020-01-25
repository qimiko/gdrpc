# Geometry Dash 2.1 Rich Presence

## A semi recreation and port of my 1.9 rich presence

### Currently features

* Official levels
  * Includes The Challenge
* Saved levels
* Editor
  * Designed to not display name
* Menu state

**Note**: This thing will probably crash if the servers go down, considering that there's many reasons for the servers to go down and many responses it will give.. Fixes will be made as time goes on...

### Using

1. inject DLL with DLL injector

### Compiling

1. import files into Visual Studio and make sure it can build a dll
2. edit `DiscordSecret.cpp` and make it yours
3. include Discord files, make sure `discord-rpc.lib` is in main folder
4. build dll
5. add the assets in the `assets` folder to discord

### TODOs

* Autoloading on game start
* Recoding main loop
* Get info without requests in case of server failure
* 🦀
