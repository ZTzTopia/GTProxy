set(CMAKE_CXX_STANDARD 20)
set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/cmake-conan/conan_provider.cmake)

cmake_minimum_required(VERSION 3.14..3.26)
project(GTProxy VERSION 1.4.1)

add_subdirectory(vendor/enet)
add_subdirectory(src)
