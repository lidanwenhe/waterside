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

set(3RDPARTY_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}/../3rdparty")

# yalantinglibs
add_definitions(-DASYNC_SIMPLE_HAS_NOT_AIO)
include_directories(${3RDPARTY_CMAKE_DIR}/yalantinglibs-main/include)
include_directories(${3RDPARTY_CMAKE_DIR}/yalantinglibs-main/include/ylt/thirdparty)

project ("waterside")

# 包含子项目。
add_subdirectory ("waterside_core")
add_subdirectory ("waterside")
add_subdirectory ("test_server")
add_subdirectory ("test_client")
