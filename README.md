# Growtopia Proxy

[![Github Action](https://img.shields.io/github/actions/workflow/status/ZTzTopia/GTProxy/cmake_ci.yml?branch=develop&logo=github&logoColor=white)](https://github.com/ZTzTopia/GTProxy/actions?query=workflow%3ACI)
[![GitHub Release](https://img.shields.io/github/v/release/ZTzTopia/GTProxy.svg?color=orange&logo=docusign&logoColor=orange)](https://github.com/ZTzTopia/GTProxy/releases/latest) 

Growtopia Proxy is a free and open-source proxy for [Growtopia](https://growtopiagame.com/), which enables the user to debug incoming and outgoing packets and even modify them. It is a useful tool for developers who want to analyze the game's network traffic and develop custom features.

> I'm just wondering if I'm having a skill issue writing this C++ code since I first started this project, or if it's because the Growtopia packet structure is pretty complicated. I've had to do a lot of rewrites and refactors. Also, the first time I made this project, it was really just for learning about networking and C++.

> I do like to add a GUI like ImGui, Qt, or WebUI, but I don't have time to do it because my own Wi-Fi IP address is flagged by the Growtopia server (Indonesian players are being stupid for real. My friend said if someone is selling residential IP addresses for botting). I will add it if I'm patient enough to find a VPN that can connect to the Growtopia server without being flagged.

## Supported Platforms

- Windows. Tested with Windows 7, 8, 10 and 11.
- GNU/Linux. Tested with Ubuntu and Arch Linux. (but who use GNU/Linux to play growtopia?)
- macOS. Tested with macOS 26 Tahoe.

## Features

- Open source and free to use.
- Support for Growtopia 3.92 and newer versions.
- Includes a built-in http server with metadata from the Growtopia client.
- Works well with sub-server redirection.
- Supports packet debugging and modification.
- Includes a configuration file for customization.
- Support for new login system.

## Download

The latest application executable can be found on the [releases page](https://github.com/ZTzTopia/GTProxy/releases).

## Build

The following dependencies are required to build from source:

- [CMake](https://cmake.org/).
- [Conan, the C/C++ Package Manager](https://conan.io) (2.0+).

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

## License

This project is licensed under the MIT License. See the [LICENSE](https://github.com/ZTzTopia/GTProxy/blob/main/LICENSE) file for details.
