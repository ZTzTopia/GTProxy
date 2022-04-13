# Growtopia Private Server
[![GitHub Release](https://img.shields.io/github/release/ZTzTopia/GTPriavteServer.svg)](https://github.com/ZTzTopia/GTPriavteServer/releases/latest) 
![Github Action](https://github.com/ZTzTopia/GTPrivateServer/actions/workflows/cmake.yml/badge.svg)

This is a private server for [Growtopia](https://growtopiagame.com/) uses [MariaDB](https://mariadb.org/) and [Redis](https://redis.io/) for its database.

## 📜 Features
- [ ] Server gateway (This is to connect to sub-servers it will use load balancer to choose which sub-servers are suitable to join).
- [ ] Load balancer (Weighted least connection algorithm and check load avg for linux).
- [ ] Sub-server (This is vertical scaling, if u have 4 cpu cores it will run 4 sub-servers. I will make horizontal scaling soon).
- [x] Database (MariaDB server or you can use MySql server).
- [ ] Anti cheat.
- [x] Player database. (Guest account)
- [x] Player inventory.
- [ ] Player stats.
- [ ] Player clothes.
- [ ] Player commands.
- [x] World database (Will check hash of all tile before saving the world to database).
- [x] World block.
- [ ] World object.
- [ ] World plant.
- [ ] World sub-server (Will check load avg before join the world).
- [x] Punch block.
- [ ] Place block.
- [ ] Drop item.
- [ ] Pick up item.

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
$ cmake ..
$ cmake --build .
```

## 📦 Running the program
- Run [MySQL](https://www.mysql.com/)/[MariaDB](https://mariadb.org/) server.
- Run [Redis](https://redis.io/) server.
- If you on windows, you need move needed dynamic binary from `/path/to/binary/conan/bin`, `path/to/binary/vendor/argon2`, and `path/to/binary/vendor/enet` to `/path/to/program/`.
- Move your latest `items.dat` to `path/to/program/data/`.
- Run the program.
- Enjoy.

## ✨ Credits
- Thanks to one of my friends helped a lot to this project.

## 🗒️ Note
Sorry if my code isnt good, im still learning C++ because I never learned through websites or anything. I dont like reading 🤣, i only read if its really necessary in my code.
