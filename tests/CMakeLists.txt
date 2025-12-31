project(GTProxy_tests)

find_package(GTest REQUIRED)

add_executable(GTProxy_tests
    utils/test_text_parse.cpp
    utils/test_byte_stream.cpp
)

target_include_directories(GTProxy_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(GTProxy_tests PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(GTProxy_tests)
