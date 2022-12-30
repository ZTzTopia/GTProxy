cmake_minimum_required(VERSION 3.14..3.24)
project(GTProxy VERSION 1.3.1)

option(ENET_BUILD_SHARED "Build shared library" ON)
add_subdirectory(vendor/enet)
add_subdirectory(src)
