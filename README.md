# Growtopia Proxy
[![Github Action](https://img.shields.io/github/actions/workflow/status/ZTzTopia/GTProxy/cmake_ci.yml?branch=develop&logo=github&logoColor=white)](https://github.com/ZTzTopia/GTProxy/actions?query=workflow%3ACI)
[![GitHub Release](https://img.shields.io/github/v/release/ZTzTopia/GTProxy.svg?color=orange&logo=docusign&logoColor=orange)](https://github.com/ZTzTopia/GTProxy/releases/latest) 

Growtopia Proxy is a free and open-source proxy for [Growtopia](https://growtopiagame.com/), which enables the user to debug incoming and outgoing packets and even modify them. It is a useful tool for developers who want to analyze the game's network traffic and develop custom features.

## Supported Platforms
- [Windows](https://www.microsoft.com/en-us/windows). Tested with Windows 7, 8, 10 and 11.
- [GNU/Linux](https://www.gnu.org/gnu/linux-and-gnu.en.html). Tested with Ubuntu and Arch Linux. (but who use GNU/Linux to play growtopia?)

## Features
- Open source and free to use.
- Support for Growtopia 3.92 and newer versions.
- Includes a built-in http server with metadata from the Growtopia client.
- Works well with sub-server redirection.
- Supports packet debugging and modification.
- Includes a configuration file for customization.

## Download
The latest application executable can be found on the [releases page](https://github.com/ZTzTopia/GTProxy/releases).

## Build
The following dependencies are required to build from source:
- [CMake](https://cmake.org/).
- [Conan, the C/C++ Package Manager](https://conan.io) (v2.0+).

To build from source:
1. Clone the source code of this project: `git clone --recurse-submodules https://github.com/ZTzTopia/GTProxy.git`
2. Install Python 3.5+ (Windows only: select 'Add Python to PATH' in installer)
3. CMake will install needed package with Conan. `pip install "conan>2.0"`
```shell
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Debug
$ cmake --build .
```

## Credits
- [Conan, the C/C++ Package Manager](https://conan.io/): The open source, decentralized and multi-platform package manager to create and share all your native binaries
- [LibreSSL](https://www.libressl.org/) is a version of the TLS/crypto stack forked from OpenSSL in 2014, with goals of modernizing the codebase, improving security, and applying best practice development processes
- [magic_enum](https://github.com/Neargye/magic_enum): Static reflection for enums (to string, from string, iteration)
- [nlohmann_json](https://github.com/nlohmann/json): JSON for Modern C++
- [pcg-cpp](https://github.com/imneme/pcg-cpp): Random number generator
- [randutils.hpp](https://gist.github.com/imneme/540829265469e673d045): Random utilities
- [spdlog](https://github.com/gabime/spdlog): Fast C++ logging library

## License
This project is licensed under the MIT License. See the [LICENSE](https://github.com/ZTzTopia/GTProxy/blob/main/LICENSE) file for details.
