cmake_minimum_required(VERSION 2.8)
project(udpclient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_WORKSPACE_DIR ${CMAKE_SOURCE_DIR}/workspace)
set(PROJECT_UNITTEST_DIR ${CMAKE_SOURCE_DIR}/unittest)

add_subdirectory(unittest)