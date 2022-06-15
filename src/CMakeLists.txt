cmake_minimum_required(VERSION 3.0..3.23)
project(Main LANGUAGES CXX)

set(GTPROXY_VERSION "${CMAKE_PROJECT_VERSION}")
if (GTPROXY_BUILD_VERSION)
    set(GTPROXY_VERSION "${GTPROXY_VERSION}+${GTPROXY_BUILD_VERSION}")
endif ()

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
    target_compile_options(${PROJECT_NAME} PRIVATE -fno-exceptions)
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

# Install all needed packages.
include(${CMAKE_CURRENT_SOURCE_DIR}/../conan/conan.cmake)

conan_cmake_configure(REQUIRES
        fmt/8.1.1
        spdlog/1.10.0
        nlohmann_json/3.10.5
        zlib/1.2.12
        libressl/3.5.3
    GENERATORS cmake_find_package
    IMPORTS "bin, *.dll -> ./bin"
    OPTIONS
        fmt:shared=True
        spdlog:shared=True
        zlib:shared=True
        libressl:shared=True)

conan_cmake_autodetect(settings)
conan_cmake_install(PATH_OR_REFERENCE .
    INSTALL_FOLDER ${CMAKE_BINARY_DIR}/conan
    BUILD missing
    REMOTE conancenter
    SETTINGS ${settings})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/conan")

find_package(fmt REQUIRED)
target_link_libraries(${PROJECT_NAME} fmt::fmt)

find_package(spdlog REQUIRED)
target_link_libraries(${PROJECT_NAME} spdlog::spdlog)

find_package(nlohmann_json REQUIRED)
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)

find_package(ZLIB REQUIRED)
target_link_libraries(${PROJECT_NAME} ZLIB::ZLIB)

find_package(LibreSSL REQUIRED)
target_link_libraries(${PROJECT_NAME} LibreSSL::SSL LibreSSL::TLS LibreSSL::Crypto)