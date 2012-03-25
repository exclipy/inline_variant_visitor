cmake_minimum_required(VERSION 2.6)
project (inline_variant_visitor)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)
set(CMAKE_CXX_FLAGS -std=c++0x)
link_directories ( ${Boost_LIBRARY_DIRS} )
add_executable(inline_variant_test
    inline_variant_test.cpp)
target_link_libraries(inline_variant_test
    boost_unit_test_framework)
