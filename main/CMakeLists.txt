cmake_minimum_required(VERSION 3.10)
project(main)

add_executable(main main.cpp)

include_directories("../window_manager")

target_link_libraries(main window_manager)