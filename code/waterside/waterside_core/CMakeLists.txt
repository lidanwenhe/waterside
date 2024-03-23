find_package(TBB REQUIRED)
find_package(Boost REQUIRED)
find_package(flatbuffers REQUIRED)

include_directories(
	${Boost_INCLUDE_DIRS}
	.
	logger
	signal
	id
	object
	time
	network
	frame
	)

file(GLOB_RECURSE ALL_FILES *.h *.cpp)
source_group_by_dir(ALL_FILES)

add_library(waterside_core STATIC ${ALL_FILES})

target_link_libraries(waterside_core flatbuffers::flatbuffers TBB::tbb TBB::tbbmalloc TBB::tbbmalloc_proxy)