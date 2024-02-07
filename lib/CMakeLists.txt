project(GTProxy-Library)

add_subdirectory(cpp-httplib)
add_subdirectory(enet)
add_subdirectory(eventpp)

option(LIBRESSL_SKIP_INSTALL "Skip installation" OFF)
option(LIBRESSL_APPS "Build apps" OFF)
option(LIBRESSL_TESTS "Build tests" ON)
add_subdirectory(libressl)
