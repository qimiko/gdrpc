# Geometry Dash 2.1 Rich Presence

## A semi recreation and port of my 1.9 rich presence

### Currently features

* Official levels
  * Includes The Challenge
* Saved levels
* Editor
* Idle state
* Autoloads
* Configuration system

**Note**: This thing might crash if servers go down.

### Using

#### Manually

1. make sure `discord-rpc.dll` is in the GD directory
2. inject `gdrpc.dll` with DLL injector
3. load the main menu (trigger `MenuLayer::init`)

#### Automatically

1. copy `zlib1.dll`, `gdrpc.dll` and `discord-rpc.dll` to the Geometry Dash directory
2. start GD

#### Configuration

The configuration file, `gdrpc.toml` is located in the Geometry Dash directory. It is created automatically on first run.
For more details on how to configurate, there is a `gdrpc.toml` file located in the root of this repository that details each of the arguments.

### Compiling

1. import files into Visual Studio as cmake project
2. download git submodules, `git submodules update --init --recursive`
3. edit `src/discord_secret.hpp` and add your application id between the quotes
4. build dll
5. add the assets in the `assets` folder to discord

#### Autoloader

The autoloader is based on a modification of the import table of `zlib1.dll`. If you want to do it yourself, just grab any generic PE header editor.

### Credits

* absolute for figurative and literal pointers
* smjs for some pointers
* nekit for the level location thingy
* cos8o for being cos8o
* blaze for some hooks
* saya for motivation
* silentzer for being smart

### Maybe

* gjgamelevel parsing for editor levels
