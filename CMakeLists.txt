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
    ${SRC_DIR}/search_server.cpp
    ${SRC_DIR}/document.cpp
    ${SRC_DIR}/read_input_functions.cpp
    ${SRC_DIR}/string_processing.cpp
)

if (DEFINED TESTS AND TESTS)
    list(APPEND LIBS ${TESTS_DIR}/server_test.cpp)
endif()

set(CFLAGS -Wall -Werror -pedantic)
set(DEFS)

if (DEFINED CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Debug") 
    message("Debug mode.")
    list(APPEND DEFS -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_LIBCPP_DEBUG=1)
    message("Debug flags: " ${DEFS})
endif()

add_executable(${PROJECT_NAME} ${LIBS})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_compile_options(${PROJECT_NAME} PRIVATE ${CFLAGS})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${DEFS})