# Growtopia Proxy
[![Github Action](https://img.shields.io/github/workflow/status/ZTzTopia/GTProxy/Build?logo=github&logoColor=white)](https://github.com/ZTzTopia/GTProxy/actions?query=workflow%3ABuild)
[![GitHub Release](https://img.shields.io/github/v/release/ZTzTopia/GTProxy.svg?color=orange&logo=docusign&logoColor=orange)](https://github.com/ZTzTopia/GTProxy/releases/latest) 

This is a proxy for [Growtopia](https://growtopiagame.com/), which makes it possible to debug incoming and outgoing packets and even modify them.

## Features
- [x] Print incoming and outgoing packets (text, variant list, etc).
- [x] Built-in http server with metadata from [Growtopia](https://growtopiagame.com/) server data.
- [x] Works well with sub-server redirection.
- [x] Command.
- [x] Config file.
- [x] Pathfinding for auto collect, etc.
- [x] Loading items' database (items.dat) will request one from [Growtopia](https://growtopiagame.com/).
- [x] Local Player & Remote player.
- [x] Auto puzzle captcha solver.

## Supported platform
- [Windows](https://www.microsoft.com/en-us/windows). Tested with Windows 10 and 11.

## Requirements
- [CMake](https://cmake.org/).
- [Conan, the C/C++ Package Manager](https://conan.io). [CMake](https://cmake.org/) will install needed package with [Conan, the C/C++ Package Manager](https://conan.io/).
Required python >= 3.5
```shell
$ pip install conan
```

## Building the source
```shell
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Debug
$ cmake --build .
```

## Running the program
- If you on Windows, you need move needed dynamic binary from `/path/to/binary/conan/bin` to `/path/to/program/`.
- Run the program.
- Edit the `config.json` file.
- Run the program again.
- Enjoy.

## Credits
- Thanks to my two friends who helped a lot with this project.
