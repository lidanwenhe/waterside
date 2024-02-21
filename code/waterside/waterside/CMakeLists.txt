# CMakeList.txt: waterside 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

find_package(TBB REQUIRED)

include_directories(
	../waterside_core
	../waterside_core/logger
	.
	)

# 将源代码添加到此项目的可执行文件。
add_executable (waterside "waterside.cpp" "waterside.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET waterside PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。

target_link_libraries(waterside PRIVATE waterside_core)