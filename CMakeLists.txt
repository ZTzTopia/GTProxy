set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CMake Conan requires this to be set or passed in via the command line
set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake-conan/conan_provider.cmake)

cmake_minimum_required(VERSION 3.24)
project(GTProxy VERSION 3.0.0)

add_subdirectory(lib)
add_subdirectory(src)

enable_testing()
add_subdirectory(tests)
