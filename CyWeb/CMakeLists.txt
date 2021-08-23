cmake_minimum_required(VERSION 3.13)
project(CyWeb)

set(CMAKE_CXX_STANDARD 14)

aux_source_directory(. ALL_ORIGIN)

add_executable(CyWeb ${ALL_ORIGIN} threadpool/threadpool.h lock/locker.h http/http_conn.h http/http_conn.cpp time/timer.cpp time/timer.h)