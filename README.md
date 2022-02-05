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

### Using

#### Manually

1. make sure `discord-rpc.dll` is in the GD directory
2. inject `gdrpc.dll` with DLL injector
3. load the main menu (trigger `MenuLayer::init`)

#### Automatically

1. Setup loader. See the [autoloader section](#autoloader) for more details.
2. Copy `gdrpc.dll` and `discord-rpc.dll` to the directory that your loader loads DLLs from.
3. start GD

#### Configuration

The configuration file, `gdrpc.toml` is located in the Geometry Dash directory. It is created automatically on first run.
For more details on how to configure, there is a `gdrpc.toml` file located in the root of this repository that details each of the arguments.

To change the presence based on the folder of the current level, add additional tables that correspond with the index of the folder that you would like to have a specific presence. `gdrpc.toml` should show more details.

### Compiling

1. import files into Visual Studio as cmake project
2. download git submodules, `git submodules update --init --recursive`
3. build dll

### Development Builds

Development builds are built through a GitHub Actions job and can be found in the [actions tab](https://github.com/qimiko/gdrpc/actions).

These builds are not tested for stability, and may crash or generally not function.  
The options format may change between commits, and could be incompatible from one commit to the next.  

Stick to [full releases](https://github.com/qimiko/gdrpc/releases) if you're unsure, as these receive more testing and a guaranteed upgrade route.

### Private servers

Private servers _might_ be supported by this, however it hasn't been actually tested.
In the latest development branch, you can edit the configuration to use a private server's executable and url. Refer to `gdrpc.toml` for more details.

If you want to use your own Discord application (to change the playing name or icon), add the files in the `assets/` folder to ensure the images show properly.

Feel free to use this rich presence on a Geometry Dash private server if wanted, but please give proper credit.

### Autoloader

At the time the project was created, automatically loading mods was not common. However, things have changed in recent years.

This project is compatible with any mod loader for Geometry Dash, from [Mega Hack v7](https://absolllute.com/store/view_mega_hack_pro) to [GDDLLLoader](https://github.com/adafcaefc/GDDLLLoader). See their instructions regarding the placement of DLLs. Note that `discord-rpc.dll` must be located in Geometry Dash's directory, alongside `GeometryDash.exe`. The mod's configuration files will also be placed in the game's directory.

Each release comes with a `zlib1.dll` based loader, which will only load the rich presence. It can be found through the releases tab or at [this link](https://github.com/qimiko/gdrpc/releases/download/2.0.0/zlib1.dll). Put both files in the directory of the game. It should replace `zlib1.dll`.

The autoloader is based on a modification of the import table of `zlib1.dll`. If you want to do it yourself, just grab any generic PE header editor.

### Credits

* absolute for figurative and literal pointers
* smjs for some pointers
* nekit for the level location thingy
* cos8o for being cos8o
* shira for some internal details
* saya for motivation
* silentzer for being smart
