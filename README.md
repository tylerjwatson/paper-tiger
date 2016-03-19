# Paper Tiger 

Paper Tiger is an experimental Terraria server written in C for OSX, WIN32 and Linux systems.

Paper Tiger is free, and released under GPL version 2, which you can find in the `LICENSE` file.

## Linux Dependencies
* `gcc`
* `cmake`

## Windows Dependencies

Building on Windows has been tested with Visual Studio 2015.

* Visual Studio 2015 with C/C++ support
* CMake for Windows

## Build instructions

* Make sure you have all the dependencies installed on your system.

### Linux

```bash
$ git clone https://github.com/tylerjwatson/paper-tiger.git
$ cd paper-tiger
$ mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
$ make
```

### Windows

* Run CMake and point it to your `paper-tiger` directory
* Tell it to generate a `Visual Studio 14` project
* Output it to the `build` folder
* Open `paper-tiger.sln` in VS
* Build and enjoy

## Running guac

`paper-tiger` (or `paper-tiger.exe` on Windows) currently only accepts the following commandline params

* `-w <path>` - Loads a world file at `<path>`
* `-s` - Silent mode, do not open or accept console commands.

