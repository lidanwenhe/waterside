find_package(Boost REQUIRED)

include_directories(
	${Boost_INCLUDE_DIRS}
	../waterside_core
	../waterside_core/logger
	../waterside_core/signal
	../waterside_core/id
	../waterside_core/object
	../waterside_core/time
	../waterside_core/network
	../waterside_core/frame
	../waterside_route
	../fbs
	.
	./dispatch
	)

file(GLOB_RECURSE ALL_FILES *.h *.cpp)
source_group_by_dir(ALL_FILES)

add_executable (GateServer ${ALL_FILES})

target_link_libraries(GateServer PRIVATE waterside_route waterside_core)

if(WIN32)
	set_target_properties(GateServer PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY $(OutDir))
endif()