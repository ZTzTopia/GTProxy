project(Proxy)

file(GLOB GTPROXY_INCLUDE_FILES
    *.hpp
    command/*.hpp
    command/commands/*.hpp
    core/*.hpp
    network/*.hpp
    packet/*.hpp
    packet/game/*.hpp
    packet/message/*.hpp
    scripting/*.hpp
    scripting/bindings/*.hpp
    utils/*.hpp)

file(GLOB GTPROXY_SOURCE_FILES
    *.cpp
    command/*.cpp
    core/*.cpp
    network/*.cpp
    packet/*.cpp
    scripting/*.cpp
    utils/*.cpp)

add_executable(${PROJECT_NAME}
    ${GTPROXY_INCLUDE_FILES}
    ${GTPROXY_SOURCE_FILES})

find_package(eventpp REQUIRED)
find_package(fmt REQUIRED)
find_package(glaze REQUIRED)
find_package(glm REQUIRED)
#find_package(httplib REQUIRED)
find_package(magic_enum REQUIRED)
find_package(pcg-cpp REQUIRED)
find_package(LibreSSL REQUIRED)
find_package(lua REQUIRED)
find_package(sol2 REQUIRED)
find_package(spdlog REQUIRED)

target_link_libraries(${PROJECT_NAME}
    enet
    eventpp::eventpp
    fmt::fmt
    glaze::glaze
    glm::glm
    httplib::httplib
    LibreSSL::TLS
    LibreSSL::Crypto
    lua::lua
    magic_enum::magic_enum
    pcg-cpp::pcg-cpp
    sol2::sol2
    spdlog::spdlog)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/enet/include)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/libressl/include)

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

# Copy the resources folder to executable directory
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/resources"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
    COMMENT "Copying ${CMAKE_SOURCE_DIR}/resources to $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources.")
