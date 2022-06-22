cmake_minimum_required(VERSION 3.14)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(PROJECT_NAME "search-server")
project(${PROJECT_NAME} CXX)

set(SRC_DIR ./search-server)
set(TESTS_DIR ${SRC_DIR}/tests)
set(LIBS 
    #${SRC_DIR}/main.cpp 
    #${SRC_DIR}/synonyms.cpp
    #${TESTS_DIR}/synonyms_test.cpp
    ./bus_manager/bus_manager.cpp
)

add_executable(${PROJECT_NAME} ${LIBS})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)