project(Proxy)

file(GLOB GTPROXY_INCLUDE_FILES
    client/*.hpp
    core/*.hpp
    domain_resolver/*.hpp
    packet/*.hpp
    player/*.hpp
    server/*.hpp
    utils/*.hpp)

file(GLOB GTPROXY_SOURCE_FILES
    *.cpp
    client/*.cpp
    core/*.cpp
    domain_resolver/*.cpp
    packet/*.cpp
    player/*.cpp
    server/*.cpp)

add_executable(${PROJECT_NAME}
    ${GTPROXY_INCLUDE_FILES}
    ${GTPROXY_SOURCE_FILES})

find_package(fmt REQUIRED)
find_package(glm REQUIRED)
find_package(httplib REQUIRED)
find_package(LibreSSL REQUIRED)
find_package(magic_enum REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(pcg-cpp REQUIRED)
find_package(spdlog REQUIRED)

target_link_libraries(${PROJECT_NAME}
    enet
    eventpp
    fmt::fmt
    glm::glm
    httplib::httplib
    LibreSSL::SSL LibreSSL::TLS LibreSSL::Crypto
    magic_enum::magic_enum
    nlohmann_json::nlohmann_json
    pcg-cpp::pcg-cpp
    spdlog::spdlog)

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE 
        /EHsc)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -fexceptions)
endif ()

target_compile_definitions(${PROJECT_NAME} PUBLIC
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    SPDLOG_FMT_EXTERNAL
    CPPHTTPLIB_OPENSSL_SUPPORT)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    GTPROXY_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR}
    GTPROXY_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR}
    GTPROXY_VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH})

if (NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        GTPROXY_DEBUG)
endif ()

# Copy dependencies binary to current binary dir.
if (WIN32)
    if (EXISTS "${CMAKE_BINARY_DIR}/conan/build/${CMAKE_BUILD_TYPE}")
        file(GLOB GTPROXY_DEPENDENCIES_BINARY
            "${CMAKE_BINARY_DIR}/conan/build/${CMAKE_BUILD_TYPE}/*.dll")
    else ()
        file(GLOB GTPROXY_DEPENDENCIES_BINARY
            "${CMAKE_BINARY_DIR}/conan/build/*.dll")
    endif ()

    foreach (ConanBinary IN LISTS GTPROXY_DEPENDENCIES_BINARY)
        string(REPLACE "${CMAKE_BINARY_DIR}/conan/build/${CMAKE_BUILD_TYPE}" "${CMAKE_CURRENT_BINARY_DIR}" REPLACED_GTPROXY_DEPENDENCIES_BINARY "${ConanBinary}")
        message(STATUS "Preparing ${ConanBinary} to ${REPLACED_GTPROXY_DEPENDENCIES_BINARY}")
        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ConanBinary}" "${REPLACED_GTPROXY_DEPENDENCIES_BINARY}"
            COMMENT "Copying ${ConanBinary} to ${REPLACED_GTPROXY_DEPENDENCIES_BINARY}")
    endforeach ()

    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:enet>" "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Copying enet binary to ${CMAKE_CURRENT_BINARY_DIR}")
endif ()

# Copy the resources folder to executable directory.
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/resources"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
    COMMENT "Copying ${CMAKE_SOURCE_DIR}/resources to $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources.")
