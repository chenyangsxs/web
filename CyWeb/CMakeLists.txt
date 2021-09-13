cmake_minimum_required(VERSION 3.12)
project(CyWeb)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -pthread)
include_directories(./include)
aux_source_directory(./src SRC_LIST)


add_executable(CyWeb ${SRC_LIST})