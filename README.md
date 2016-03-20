# Paper Tiger 

Paper Tiger is an experimental Terraria server written in C for OSX, WIN32 and Linux systems.

Paper Tiger aims to implement as much as possible of the official server from Re-Logic, in an efficient and cross-platform manner, with included extras such as scripting support, plugin support, and extra security features.  Paper Tiger is designed to not require any external third-party runtimes on Linux or OSX (such as `mono`), and is packagable and runnable as a system service.

Paper Tiger is designed for the official Terraria client, and does _not_ support third-party, or unlicensed Terraria clients.

Paper Tiger is free, and released under the GPLv2, which you can find in the `LICENSE` file.

## Technical Information

Paper tiger is written in a mix of C and C++ and is heavily based on the event-loop paradigm based on [libuv](https://github.com/libuv/libuv).  

libuv provides a cross-platform event loop mechanism, and Paper Tiger leverages this feature to efficiently implement a gameserver in a cross-platform manner without having to deal with threading between POSIX and Win32 environments.  The game's event loop (see `struct game`) is responsible for handling stream I/O from all the connected clients as well as the game update routines and console input.

The software is primarily written in in C, however there are some parts of the system which leverate c++11 features. One such feature is the parameter parser (see `src/param.cc`) where a cross-platform regex implementation was needed. 

## Platforms

Paper Tiger has been tested to build run on Linux (Fedora 23), MacOSX 10.10, and Windows 10. 

## Building Paper Tiger

Paper Tiger currently has been tested to build on MacOSX, Linux, and Windows.  Building for all platforms requires [CMake](https://cmake.org/).

**Note:** Building on windows _does not_ require MINGW or Cygwin.

### Building on Linux

* Install the build dependencies: `make`, `cmake`, `gcc-c++`, `libuv-devel`
* Clone the source: `git clone https://github.com/tylerjwatson/paper-tiger.git`
* Use CMake to build:

```bash

$ cd paper-tiger
$ mkdir build && cd build
$ cmake ..
$ make

```

### Building on OSX

* Install the build dependencies. `cmake`, `libuv`.  I used homebrew for this.
* Install, and accept the license terms for XCode
* Clone the source: `git clone https://github.com/tylerjwatson/paper-tiger.git`
* Open `Terminal.app`, and use CMake to build:

```bash

$ cd paper-tiger
$ mkdir build && cd build
$ cmake -G XCode ..
$ xcodebuild

```

### Building on Windows

* Install Visual Studio 2015 Community with C++ support.
  * _Note:_ MFC is not required.


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

## Running Paper Tiger

`paper-tiger` (or `paper-tiger.exe` on Windows) currently only accepts the following commandline params

* `-w <path>` - Loads a world file at `<path>`
* `-s` - Silent mode, do not open or accept console commands.

