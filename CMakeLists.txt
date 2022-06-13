cmake_minimum_required(VERSION 3.14)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(PROJECT_NAME "search-server")
project(${PROJECT_NAME} CXX)

set(SRC_DIR "./search-server")
set(LIBS ${SRC_DIR}/main.cpp)

add_executable(${PROJECT_NAME} ${LIBS})