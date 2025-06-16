cmake_minimum_required(VERSION 3.10)
project(flappybird C)

set(CMAKE_C_STANDARD 99)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(flappybird main.c)
target_link_libraries(flappybird ${CURSES_LIBRARIES})
