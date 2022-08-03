cmake_minimum_required(VERSION 3.14)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(PROJECT_NAME "transport-catalogue")
project(${PROJECT_NAME} CXX)

set(SRC_DIR ./transport-catalogue)
set(MAIN_FILE ${SRC_DIR}/main.cpp )

if (DEFINED TESTS AND TESTS)
    set(TESTS_DIR ${SRC_DIR}/tests)
    set(MAIN_FILE ${TESTS_DIR}/main.cpp )
endif()

set(LIBS 
    ${MAIN_FILE}
    ${SRC_DIR}/transport_catalogue.cpp
    ${SRC_DIR}/input_reader.cpp
    ${SRC_DIR}/stat_reader.cpp
    ${SRC_DIR}/input_reader.cpp
)

if (DEFINED TESTS AND TESTS)
    list(APPEND LIBS ${TESTS_DIR}/server_test.cpp)
endif()

set(CFLAGS -Wall -Werror -pedantic)
set(DEFS)
set(LINK_OPTS)
set(LDLIBS tbb pthread)

if (DEFINED CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Debug") 
    message("Debug mode.")
    list(APPEND DEFS -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_LIBCPP_DEBUG=1)
    message("Debug flags: " ${DEFS})
    if (DEFINED FSANITIZE_DEBUG AND FSANITIZE_DEBUG)
        list(APPEND CFLAGS -fsanitize=address -fsanitize=leak -O1 -fno-omit-frame-pointer -fno-optimize-sibling-calls)
        list(APPEND LINK_OPTS -fsanitize=address -fsanitize=leak -O1 -fno-omit-frame-pointer -fno-optimize-sibling-calls)
        message("FSANITIZE_DEBUG" enabled)
    endif()
endif()

add_executable(${PROJECT_NAME} ${LIBS})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_compile_options(${PROJECT_NAME} PRIVATE ${CFLAGS})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${DEFS})
target_link_options(${PROJECT_NAME} PRIVATE ${LINK_OPTS})
target_link_libraries(${PROJECT_NAME} ${LDLIBS})