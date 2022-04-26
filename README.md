# Growtopia Proxy
[![GitHub Release](https://img.shields.io/github/release/ZTzTopia/GTPriavteServer.svg)](https://github.com/ZTzTopia/GTPriavteServer/releases/latest) 
![Github Action](https://github.com/ZTzTopia/GTPrivateServer/actions/workflows/cmake.yml/badge.svg)

This is a proxy for [Growtopia](https://growtopiagame.com/).

## 📜 Features
- [x] Proxy for Growtopia.

## 💻 Supported platforms
- [Windows](https://www.microsoft.com/en-us/windows). Tested with Windows 11.
- [GNU/Linux](https://www.gnu.org/gnu/linux-and-gnu.en.html). Tested with Arch WSL, not 100% tested.

## 📝 Requirements
- [CMake](https://cmake.org/).
- [Conan, the C/C++ Package Manager](https://conan.io). [CMake](https://cmake.org/) will install needed package with [Conan, the C/C++ Package Manager](https://conan.io/).
Required python >= 3.5
```shell
$ pip install conan
```

## ⚙️ Setting up
- Rename `src/config.sample.h` to `src/config.h` and fill it.

## 🔨 Building the source
```shell
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=DEBUG
$ cmake --build .
```

## 📦 Running the program
- If you on windows, you need move needed dynamic binary from `/path/to/binary/conan/bin` to `/path/to/program/`.
- Run the program.
- Enjoy.

## ✨ Credits
- Thanks to one of my friends helped a lot to this project.
