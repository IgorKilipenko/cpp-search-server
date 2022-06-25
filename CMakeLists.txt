cmake_minimum_required(VERSION 3.14)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(PROJECT_NAME "search-server")
project(${PROJECT_NAME} CXX)

set(SRC_DIR ./search-server)
set(MAIN_FILE ${SRC_DIR}/main.cpp )

set(LIBS 
    ${MAIN_FILE}
)

set(CFLAGS -Wall -Werror -pedantic)

add_executable(${PROJECT_NAME} ${LIBS})
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSERVER_TEST=1")
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_compile_options(${PROJECT_NAME} PRIVATE ${CFLAGS})