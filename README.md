# upgraded-guacamole
Upgraded guacamole is an experimental Terraria server written in C for POSIX-based operating systems.

Upgraded guacamole is free, and released under GPL version 2, which you can find in the `LICENSE` file.

## Dependencies
* `gcc`
* `cmake`
* `libtalloc-devel`

## Build instructions

* Make sure you have all the dependencies installed on your system.

```bash
$ git clone https://github.com/tylerjwatson/upgraded-guacamole.git
$ cd upgraded-guacamole
$ cmake -DCMAKE_BUILD_TYPE=Debug .
$ make
```

## Run instructions

`make` should generate an `upgraded-guacamole` executable.  Feel free to run it.
