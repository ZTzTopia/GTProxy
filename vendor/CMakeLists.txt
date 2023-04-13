project(Vendor)

add_subdirectory(enet)

if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/develop/conan.cmake"
        "${CMAKE_BINARY_DIR}/conan/conan.cmake"
        TLS_VERIFY ON)
endif ()

include(${CMAKE_BINARY_DIR}/conan/conan.cmake)

set(LIBS)
list(APPEND LIBS "cpp-httplib/0.11.2 ")
list(APPEND LIBS "fmt/8.1.1 ")
list(APPEND LIBS "libressl/3.5.3 ")
list(APPEND LIBS "magic_enum/0.8.1 ")
list(APPEND LIBS "nlohmann_json/3.11.2 ")
list(APPEND LIBS "pcg-cpp/cci.20210406 ")
list(APPEND LIBS "spdlog/1.10.0 ")
list(APPEND LIBS "zlib/1.2.13 ")

set(LIB_OPTIONS)
list(APPEND LIB_OPTIONS "libressl:shared=True ")

conan_cmake_configure(REQUIRES ${LIBS}
    GENERATORS cmake_find_package
    IMPORTS "bin, * -> ./bin"
    IMPORTS "lib, * -> ./lib"
    OPTIONS ${LIB_OPTIONS})

conan_cmake_autodetect(settings)
conan_cmake_install(PATH_OR_REFERENCE ${CMAKE_CURRENT_BINARY_DIR}
    INSTALL_FOLDER ${CMAKE_BINARY_DIR}/conan
    BUILD missing
    REMOTE conancenter
    SETTINGS ${settings})
