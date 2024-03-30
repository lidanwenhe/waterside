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
	../waterside_mysql
	../fbs
	.
	./dispatch
	./mysql
	)

file(GLOB_RECURSE ALL_FILES *.h *.cpp)
source_group_by_dir(ALL_FILES)

add_executable (LoginServer ${ALL_FILES})

target_link_libraries(LoginServer PRIVATE waterside_mysql waterside_route)

if(WIN32)
	set_target_properties(LoginServer PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY $(OutDir))
endif()