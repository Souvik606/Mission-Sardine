cmake_minimum_required(VERSION 3.23)
project(MissionSardine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(MissionSardine
        shell.cpp
        data_types/user_function_type.cpp
        ast_nodes/string_nodes.h
        data_types/string_type.h
        ast_nodes/list_nodes.h
        data_types/list_type.h
        ast_nodes/dict_nodes.h
        data_types/dict_type.h
        data_types/builtins.h)