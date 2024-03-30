find_package(Boost REQUIRED)
find_package(flatbuffers REQUIRED)

include_directories(
	${Boost_INCLUDE_DIRS}
	../common/include
	.
	)

file(GLOB_RECURSE ALL_FILES *.h *.cpp)
source_group_by_dir(ALL_FILES)

add_executable (stmtc ${ALL_FILES})

target_link_libraries(stmtc PRIVATE flatbuffers::flatbuffers)

if(WIN32)
	set_target_properties(stmtc PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY $(OutDir))
endif()
