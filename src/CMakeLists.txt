project(Proxy)

file(GLOB GTPROXY_INCLUDE_FILES
    client/*.hpp
    core/*.hpp
    packet/*.hpp
    extension/*.hpp
    extension/**/*.hpp
    packet/game/*.hpp
    packet/message/*.hpp
    player/*.hpp
    server/*.hpp
    utils/*.hpp)

file(GLOB GTPROXY_SOURCE_FILES
    *.cpp
    client/*.cpp
    core/*.cpp
    extension/*.cpp
    extension/**/*.cpp
    packet/*.cpp
    player/*.cpp
    server/*.cpp
    utils/*.cpp)

add_executable(${PROJECT_NAME}
    ${GTPROXY_INCLUDE_FILES}
    ${GTPROXY_SOURCE_FILES})

# Try to find the required packages
find_package(fmt REQUIRED)
find_package(glm REQUIRED)
# find_package(httplib REQUIRED)
find_package(magic_enum REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(pcg-cpp REQUIRED)
find_package(spdlog REQUIRED)

# Target the required packages
target_link_libraries(${PROJECT_NAME}
    enet
    eventpp
    crypto
    tls
    fmt::fmt
    glm::glm
    httplib::httplib
    magic_enum::magic_enum
    nlohmann_json::nlohmann_json
    pcg-cpp::pcg-cpp
    spdlog::spdlog)

# ?????????
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

# Set the version number
target_compile_definitions(${PROJECT_NAME} PRIVATE
    GTPROXY_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR}
    GTPROXY_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR}
    GTPROXY_VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH})

# Enable spdlog debug
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
