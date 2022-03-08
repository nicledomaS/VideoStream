cmake_minimum_required(VERSION 3.10)

project(http_streamer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

shared_library(${PROJECT_NAME}
    SOURCES "src/"
    THREADS
    CONAN_MODULES
    #BOOST system
    MODULES GSL)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)