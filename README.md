# Growtopia Proxy
[![Github Action](https://img.shields.io/github/workflow/status/ZTzTopia/GTProxy/Build?logo=github&logoColor=white)](https://github.com/ZTzTopia/GTProxy/actions?query=workflow%3ABuild)
[![GitHub Release](https://img.shields.io/github/v/release/ZTzTopia/GTProxy.svg?color=orange&logo=docusign&logoColor=orange)](https://github.com/ZTzTopia/GTProxy/releases/latest) 

This is a proxy for [Growtopia](https://growtopiagame.com/), which makes it possible to debug incoming and outgoing packets and even modify them.

## Features
- [x] Print incoming and outgoing packets (text, variant list, etc).
- [x] Support for [Growtopia](https://growtopiagame.com/) 3.92 and up.
- [x] Built-in http server with metadata from [Growtopia](https://growtopiagame.com/) server data.
- [x] Works well with sub-server redirection.
- [ ] Command.
- [x] Config file.
- [ ] Pathfinding for auto collect, etc.
- [ ] Loading items' database (items.dat) will request one from [Growtopia](https://growtopiagame.com/).
- [ ] Local Player & Remote player.
- [ ] Auto puzzle captcha solver.

## Supported platform
- [Windows](https://www.microsoft.com/en-us/windows). Tested with Windows 10 and 11.

## Structure
```
- src (The source file)
    - client (client to communicate with growtopia server)
    - enetwrapper
    - include
        - pcg (random generator like mt19937)
    - player
    - server (server to communicate with growtopia client)
    - utils
- vendor
    - enet
    - proton
```

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

## For Growtopia version 3.91-4.03
- You need add `www.` before growtopia[1-2].com in `hosts` file.
```text
127.0.0.1 www.growtopia1.com
127.0.0.1 www.growtopia2.com
```

## Running the program
- If you on Windows, you need move needed dynamic binary from `/path/to/binary/conan/bin` to `/path/to/program/`.
- Run the program.
- Edit the `config.json` file.
- Run the program again.
- Enjoy.

## Credits
- Thanks to my two friends who helped a lot with this project.
- [Conan, the C/C++ Package Manager](https://conan.io/): The open source, decentralized and multi-platform package manager to create and share all your native binaries- [LibreSSL](https://www.libressl.org/) is a version of the TLS/crypto stack forked from OpenSSL in 2014, with goals of modernizing the codebase, improving security, and applying best practice development processes
- [magic_enum](https://github.com/Neargye/magic_enum): Static reflection for enums (to string, from string, iteration)
- [nlohmann_json](https://github.com/nlohmann/json) JSON for Modern C++
- [pcg-cpp](https://github.com/imneme/pcg-cpp): Random number generator
- [randutils.hpp](https://gist.github.com/imneme/540829265469e673d045): Random utilities
- [spdlog](https://github.com/gabime/spdlog): Fast logging library
