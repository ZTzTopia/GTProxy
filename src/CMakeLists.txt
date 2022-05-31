cmake_minimum_required(VERSION 3.0..3.23)
project(Main LANGUAGES CXX)

file(GLOB INCLUDE_FILES
    *.h
    client/*.h
    command/*.h
    enetwrapper/*.h
    include/*.hpp
    include/pcg/*.hpp
    items/*.h
    player/*.h
    server/*.h
    utils/*.h
    world/*.h
    ../vendor/proton/shared/util/*.h)

file(GLOB SOURCE_FILES
    *.cpp
    client/*.cpp
    command/*.cpp
    enetwrapper/*.cpp
    player/*.cpp
    server/*.cpp
    ../vendor/proton/shared/util/*.cpp)

add_executable(${PROJECT_NAME}
    ${INCLUDE_FILES}
    ${SOURCE_FILES})

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

if (GTPROXY_BUILD_VERSION)
    set(GTPROXY_VERSION "${CMAKE_PROJECT_VERSION}+${GTPROXY_BUILD_VERSION}")
else ()
    set(GTPROXY_VERSION "${CMAKE_PROJECT_VERSION}")
endif ()

target_compile_definitions(${PROJECT_NAME} PUBLIC
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    SPDLOG_FMT_EXTERNAL
    GTPROXY_VERSION="${GTPROXY_VERSION}")

target_link_libraries(${PROJECT_NAME} enet_static)

# Install all needed packages.
include(${CMAKE_CURRENT_SOURCE_DIR}/../conan/conan.cmake)

conan_cmake_configure(REQUIRES
        fmt/8.1.1
        spdlog/1.10.0
        cpp-httplib/0.10.7
        nlohmann_json/3.10.5
        zlib/1.2.12
    GENERATORS cmake_find_package
    IMPORTS "bin, *.dll -> ./bin"
    OPTIONS
        fmt:shared=True
        spdlog:shared=True
        zlib:shared=True)

conan_cmake_autodetect(settings)
conan_cmake_install(PATH_OR_REFERENCE .
    INSTALL_FOLDER ${CMAKE_BINARY_DIR}/conan
    BUILD missing
    REMOTE conancenter
    SETTINGS ${settings})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/conan")

find_package(fmt REQUIRED)
if (fmt_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC ${fmt_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${fmt_LIBRARIES})
endif ()

find_package(spdlog REQUIRED)
if (spdlog_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC ${spdlog_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${spdlog_LIBRARIES})
endif ()

find_package(httplib REQUIRED)
if (httplib_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC ${httplib_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${httplib_LIBRARIES})
endif ()

find_package(nlohmann_json REQUIRED)
if (nlohmann_json_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC ${nlohmann_json_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${nlohmann_json_LIBRARIES})
endif ()

find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC ${ZLIB_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES})
endif ()
