cmake_minimum_required(VERSION 3.23)
project(MissionSardine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32 AND NOT MSVC)
    add_compile_options(-Wa,-mbig-obj)
endif()

add_executable(MissionSardine
        shell.cpp
        data_types/data_type.cpp
        data_types/user_function_type.cpp
        data_types/model_type.cpp
        ast_nodes/string_nodes.h
        data_types/string_type.h
        ast_nodes/list_nodes.h
        data_types/list_type.h
        ast_nodes/dict_nodes.h
        data_types/dict_type.h
        data_types/builtins.h
        ast_nodes/fstring_nodes.h
        ast_nodes/foreach_nodes.h
        ast_nodes/summon_nodes.h
        data_types/module_type.h
        data_types/super_proxy.h
        data_types/file_type.h
        data_types/null_type.h)