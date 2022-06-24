cmake_minimum_required(VERSION 3.14)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(PROJECT_NAME "search-server")
project(${PROJECT_NAME} CXX)

set(SRC_DIR ./search-server)
set(MAIN_FILE ${SRC_DIR}/main.cpp )

if (DEFINED TESTS AND TESTS)
    set(TESTS_DIR ./search-server/tests)
    set(MAIN_FILE ${TESTS_DIR}/main.cpp )
endif()

set(LIBS 
    ${MAIN_FILE}
    ${SRC_DIR}/server.cpp
)

if (DEFINED TESTS AND TESTS)
    list(APPEND LIBS ${TESTS_DIR}/server_test.cpp)
endif()

add_executable(${PROJECT_NAME} ${LIBS})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)