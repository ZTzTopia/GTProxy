cmake_minimum_required(VERSION 3.0..3.24)
project(Source LANGUAGES CXX)

set(GTPROXY_VERSION "${CMAKE_PROJECT_VERSION}")
if (GTPROXY_BUILD_VERSION)
    set(GTPROXY_VERSION "${GTPROXY_VERSION}+${GTPROXY_BUILD_VERSION}")
endif ()
message(STATUS "GTProxy version: ${GTPROXY_VERSION}")

file(GLOB GTPROXY_INCLUDE_FILES
        *.h
        **/*.h
        **/*.hpp
        **/**/*.hpp
        ../vendor/proton/**/*.h
        ../vendor/proton/**/**/*.h)

file(GLOB GTPROXY_SOURCE_FILES
        *.cpp
        **/*.cpp
        ../vendor/proton/**/**/*.cpp)

add_executable(${PROJECT_NAME}
        ${GTPROXY_INCLUDE_FILES}
        ${GTPROXY_SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /EHsc)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -fexceptions)
endif ()

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/proton/shared)

target_compile_definitions(${PROJECT_NAME} PUBLIC
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        SPDLOG_FMT_EXTERNAL
        CPPHTTPLIB_OPENSSL_SUPPORT
        GTPROXY_VERSION="${GTPROXY_VERSION}")

target_link_libraries(${PROJECT_NAME} enet)

if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/develop/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan/conan.cmake"
            TLS_VERIFY ON)
endif ()

include(${CMAKE_BINARY_DIR}/conan/conan.cmake)

conan_cmake_configure(REQUIRES cpp-httplib/0.10.8
        GENERATORS cmake_find_package)

conan_cmake_configure(REQUIRES fmt/8.1.1
        GENERATORS cmake_find_package
        IMPORTS "bin, * -> ./bin"
        OPTIONS fmt:shared=True)

conan_cmake_configure(REQUIRES libressl/3.5.3
        GENERATORS cmake_find_package
        IMPORTS "bin, * -> ./bin"
        OPTIONS libressl:shared=True)

conan_cmake_configure(REQUIRES magic_enum/0.8.0
        GENERATORS cmake_find_package)

conan_cmake_configure(REQUIRES nlohmann_json/3.10.5
        GENERATORS cmake_find_package)

conan_cmake_configure(REQUIRES spdlog/1.10.0
        GENERATORS cmake_find_package
        IMPORTS "bin, * -> ./bin"
        OPTIONS spdlog:shared=True)

conan_cmake_configure(REQUIRES zlib/1.2.12
        GENERATORS cmake_find_package
        IMPORTS "bin, * -> ./bin"
        OPTIONS zlib:shared=True)

conan_cmake_autodetect(settings)
conan_cmake_install(PATH_OR_REFERENCE ${CMAKE_CURRENT_BINARY_DIR}
        INSTALL_FOLDER ${CMAKE_BINARY_DIR}/conan
        BUILD missing
        REMOTE conancenter
        SETTINGS ${settings})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/conan")

find_package(fmt REQUIRED)
find_package(httplib REQUIRED)
find_package(LibreSSL REQUIRED)
find_package(magic_enum REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(ZLIB REQUIRED)

target_link_libraries(${PROJECT_NAME}
        fmt::fmt
        httplib::httplib
        LibreSSL::SSL LibreSSL::TLS LibreSSL::Crypto
        magic_enum::magic_enum
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        ZLIB::ZLIB)

# Create symlink file to current binary dir.
file(GLOB GTPROXY_CONAN_BINARY
        "${CMAKE_BINARY_DIR}/conan/bin/*.dll")

foreach(ConanBinary IN LISTS GTPROXY_CONAN_BINARY)
    string(REPLACE "${CMAKE_BINARY_DIR}/conan/bin" "${CMAKE_CURRENT_BINARY_DIR}" REPLACED_CONAN_BINARY "${ConanBinary}")
    add_custom_command(
            TARGET ${PROJECT_NAME} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink "${ConanBinary}" "${REPLACED_CONAN_BINARY}"
            COMMENT "Creating symlink conan binary: ${ConanBinary} to ${REPLACED_CONAN_BINARY}")
endforeach()

