cmake_minimum_required(VERSION 3.23)
project(MissionSardine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(MissionSardine
        shell.cpp
        data_types/user_function_type.cpp)