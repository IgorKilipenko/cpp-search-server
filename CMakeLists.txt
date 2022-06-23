cmake_minimum_required(VERSION 3.14)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(PROJECT_NAME "search-server")
project(${PROJECT_NAME} CXX)

set(SRC_DIR ./search-server)
set(TESTS_DIR ./search-server/tests)
set(LIBS 
    ${SRC_DIR}/main.cpp 
    ${SRC_DIR}/server.cpp
    ${TESTS_DIR}/server_test.cpp
)

add_executable(${PROJECT_NAME} ${LIBS})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)