enable_testing()

add_executable(basic_test basic_test.cc)
target_link_libraries(basic_test PRIVATE gtest gtest_main)
target_include_directories(basic_test PUBLIC ../include)

add_test(basic_test basic_test)
