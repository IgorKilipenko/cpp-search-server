cmake_minimum_required(VERSION 3.14)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(PROJECT_NAME "search-server")
project(${PROJECT_NAME} CXX)

set(SRC_DIR ./search-server)
set(TRAINING_DIR ./training)
set(LIBS 
    ${SRC_DIR}/main.cpp 
    #${TRAINING_DIR}/s1_t_4_6.cpp 
    #${TRAINING_DIR}/section_1/topic_6/s1_t_6.cpp
    ${TRAINING_DIR}/section_1/topic_7/s1_t_7.cpp
)

add_executable(${PROJECT_NAME} ${LIBS})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)