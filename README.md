# upgraded-guacamole
Upgraded guacamole is an experimental Terraria server written in C for OSX, WIN32 and Linux systems.

Upgraded guacamole is free, and released under GPL version 2, which you can find in the `LICENSE` file.

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
$ git clone https://github.com/tylerjwatson/upgraded-guacamole.git
$ cd upgraded-guacamole
$ mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
$ make
```

### Windows

* Run CMake and point it to your `upgraded-guacamole` directory
* Tell it to generate a `Visual Studio 14` project
* Output it to the `build` folder
* Open `upgraded-guacamole.sln` in VS
* Build and enjoy

## Running guac

`guac` (or `guac.exe` on Windows) currently only accepts the following commandline params

* `-w <path>` - Loads a world file at `<path>`

