# Geometry Dash 1.9 Rich Presence

## Port of a recreation of my 1.9 rich presence

### Why..?

The purpose of the experiment branch is to test porting to new versions along with testing of GDPS support. 1.9 is used as a simple version that I have high familiarity with.

### Currently features

* Official levels
  * Includes The Challenge
* Saved levels
* Editor
* Idle state
* Autoloads
* Configuration system

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
For more details on how to configure, there is a `gdrpc.toml` file located in the root of this repository that details each of the arguments.

### Compiling

1. import files into Visual Studio as cmake project
2. download git submodules, `git submodules update --init --recursive`
3. build dll

### Private servers

Private servers _might_ be supported by this, however it hasn't been actually tested.
In the latest development branch, you can edit the configuration to use a private server's executable and url. Refer to `gdrpc.toml` for more details.

If you want to use your own Discord application (to change the playing name or icon), add the files in the `assets/` folder to ensure the images show properly.

Feel free to use this rich presence on a Geometry Dash private server if wanted, but please give proper credit.

#### Autoloader

The autoloader is based on a modification of the import table of `zlib1.dll`. If you want to do it yourself, just grab any generic PE header editor.

### Credits

* absolute for figurative and literal pointers
* smjs for some pointers
* nekit for the level location thingy
* cos8o for being cos8o
* shira for some internal details
* saya for motivation
* silentzer for being smart

### Maybe

* object count in levels