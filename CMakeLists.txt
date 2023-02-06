cmake_minimum_required(VERSION 3.12)
project(bashapp)

set(CMAKE_CXX_STANDARD 20)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(
  ${PROJECT_NAME}
  src/main.cpp
)

target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARY})