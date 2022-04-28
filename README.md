# Growtopia Proxy
[![GitHub Release](https://img.shields.io/github/release/ZTzTopia/GTProxy.svg)](https://github.com/ZTzTopia/GTProxy/releases/latest) 
![Github Action](https://github.com/ZTzTopia/GTProxy/actions/workflows/CMake.yml/badge.svg)

This is a proxy for [Growtopia](https://growtopiagame.com/).

## 📜 Features
- [x] Proxy for Growtopia.
- [x] Command.
- [x] Config.

## 💻 Supported platforms
- [Windows](https://www.microsoft.com/en-us/windows). Tested with Windows 11.

## 📝 Requirements
- [CMake](https://cmake.org/).
- [Conan, the C/C++ Package Manager](https://conan.io). [CMake](https://cmake.org/) will install needed package with [Conan, the C/C++ Package Manager](https://conan.io/).
Required python >= 3.5
```shell
$ pip install conan
```

## 🔨 Building the source
```shell
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Debug
$ cmake --build .
```

## 📦 Running the program
- If you on Windows, you need move needed dynamic binary from `/path/to/binary/conan/bin` to `/path/to/program/`.
- Run the program.
- Edit the `config.json` file.
- Run the program again.
- Enjoy.

## ✨ Credits
- Thanks to my two friends who helped a lot with this project.
