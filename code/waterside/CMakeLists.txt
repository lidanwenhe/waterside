# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.27)

# use c++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

if(MSVC)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/lib/${presetName}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/lib/${presetName}")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../../bin/${presetName}")
else(MSVC)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/lib/${CMAKE_BUILD_TYPE}/${presetName}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/lib/${CMAKE_BUILD_TYPE}/${presetName}")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../../bin/${CMAKE_BUILD_TYPE}/${presetName}")
endif(MSVC)
		  
set(3RDPARTY_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}/../3rdparty")

# yalantinglibs
add_definitions(-DASYNC_SIMPLE_HAS_NOT_AIO)
include_directories(${3RDPARTY_CMAKE_DIR}/yalantinglibs-main/include)
include_directories(${3RDPARTY_CMAKE_DIR}/yalantinglibs-main/include/ylt/thirdparty)

macro(source_group_by_dir source_files)
    if(MSVC)
        set(sgbd_cur_dir ${CMAKE_CURRENT_SOURCE_DIR})
        foreach(sgbd_file ${${source_files}})
            string(REGEX REPLACE ${sgbd_cur_dir}/\(.*\) \\1 sgbd_fpath ${sgbd_file})
            string(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
            string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
            string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
            if(sgbd_nogroup)
                set(sgbd_group_name "\\")
            endif(sgbd_nogroup)
            source_group(${sgbd_group_name} FILES ${sgbd_file})
        endforeach(sgbd_file)
    endif(MSVC)
endmacro(source_group_by_dir)

project ("waterside")

# 包含子项目。
add_subdirectory ("waterside_core")
add_subdirectory ("waterside_route")
add_subdirectory ("waterside_mysql")
add_subdirectory ("NameRegServer")
add_subdirectory ("LoginServer")
add_subdirectory ("GameServer")
add_subdirectory ("test_server")
add_subdirectory ("test_client")
