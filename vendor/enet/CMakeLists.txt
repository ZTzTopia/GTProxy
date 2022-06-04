cmake_minimum_required(VERSION 2.8.12...3.23)
project(enet)

set(ENET_MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(ENET_MAIN_PROJECT ON)
endif ()

option(ENET_BUILD_SHARED "Build shared library" OFF)

# The "configure" step.
include(CheckFunctionExists)
include(CheckStructHasMember)
include(CheckTypeSize)
check_function_exists("fcntl" HAS_FCNTL)
check_function_exists("poll" HAS_POLL)
check_function_exists("getaddrinfo" HAS_GETADDRINFO)
check_function_exists("getnameinfo" HAS_GETNAMEINFO)
check_function_exists("gethostbyname_r" HAS_GETHOSTBYNAME_R)
check_function_exists("gethostbyaddr_r" HAS_GETHOSTBYADDR_R)
check_function_exists("inet_pton" HAS_INET_PTON)
check_function_exists("inet_ntop" HAS_INET_NTOP)
check_struct_has_member("struct msghdr" "msg_flags" "sys/types.h;sys/socket.h" HAS_MSGHDR_FLAGS)
set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h" "sys/socket.h")
check_type_size("socklen_t" HAS_SOCKLEN_T BUILTIN_TYPES_ONLY)
unset(CMAKE_EXTRA_INCLUDE_FILES)
if(MSVC)
	add_definitions(-W3)
else()
	add_definitions(-Wno-error)
endif()

if(HAS_FCNTL)
    add_definitions(-DHAS_FCNTL=1)
endif()
if(HAS_POLL)
    add_definitions(-DHAS_POLL=1)
endif()
if(HAS_GETNAMEINFO)
    add_definitions(-DHAS_GETNAMEINFO=1)
endif()
if(HAS_GETADDRINFO)
    add_definitions(-DHAS_GETADDRINFO=1)
endif()
if(HAS_GETHOSTBYNAME_R)
    add_definitions(-DHAS_GETHOSTBYNAME_R=1)
endif()
if(HAS_GETHOSTBYADDR_R)
    add_definitions(-DHAS_GETHOSTBYADDR_R=1)
endif()
if(HAS_INET_PTON)
    add_definitions(-DHAS_INET_PTON=1)
endif()
if(HAS_INET_NTOP)
    add_definitions(-DHAS_INET_NTOP=1)
endif()
if(HAS_MSGHDR_FLAGS)
    add_definitions(-DHAS_MSGHDR_FLAGS=1)
endif()
if(HAS_SOCKLEN_T)
    add_definitions(-DHAS_SOCKLEN_T=1)
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

set(INCLUDE_FILES_PREFIX include/enet)
set(INCLUDE_FILES
    ${INCLUDE_FILES_PREFIX}/callbacks.h
    ${INCLUDE_FILES_PREFIX}/enet.h
    ${INCLUDE_FILES_PREFIX}/list.h
    ${INCLUDE_FILES_PREFIX}/protocol.h
    ${INCLUDE_FILES_PREFIX}/time.h
    ${INCLUDE_FILES_PREFIX}/types.h
    ${INCLUDE_FILES_PREFIX}/unix.h
    ${INCLUDE_FILES_PREFIX}/utility.h
    ${INCLUDE_FILES_PREFIX}/win32.h
)

set(SOURCE_FILES
    callbacks.c
    compress.c
    host.c
    list.c
    packet.c
    peer.c
    protocol.c
    unix.c
    win32.c
)

source_group(include FILES ${INCLUDE_FILES})
source_group(source FILES ${SOURCE_FILES})

if(ENET_BUILD_SHARED OR BUILD_SHARED_LIBS)
    add_library(enet SHARED
        ${INCLUDE_FILES}
        ${SOURCE_FILES}
    )
else ()
    add_library(enet STATIC
        ${INCLUDE_FILES}
        ${SOURCE_FILES}
    )
endif ()

target_compile_definitions(enet PRIVATE _WINSOCK_DEPRECATED_NO_WARNINGS)

if (ENET_BUILD_SHARED OR BUILD_SHARED_LIBS)
    if (MSVC)
        target_compile_definitions(enet
            PRIVATE
                ENET_DLL=1
                ENET_BUILDING_LIB=1
        )
    endif()
endif()

target_include_directories(enet
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

if (WIN32)
    target_link_libraries(enet PRIVATE winmm ws2_32)
endif()

# Install enet binary.
if (ENET_MAIN_PROJECT)
    include(GNUInstallDirs)

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(TARGETS enet
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif ()
