cmake_minimum_required(VERSION 3.10)

project(basic_wm VERSION 0.1.0)

include(CTest)
enable_testing()

include(GoogleTest)
enable_testing()

add_subdirectory(main)
add_subdirectory(window_manager)