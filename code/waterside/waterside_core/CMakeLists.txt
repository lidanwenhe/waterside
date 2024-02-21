project(waterside_core)

find_package(TBB REQUIRED)

include_directories(
	.
	logger
	signal
	id
	object
	time
	network
	)

file(GLOB_RECURSE ALL_FILES *.cpp)

add_library(waterside_core STATIC ${ALL_FILES})

target_link_libraries(waterside_core TBB::tbb TBB::tbbmalloc TBB::tbbmalloc_proxy)