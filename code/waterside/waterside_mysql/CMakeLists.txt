find_package(Boost REQUIRED)
find_package(libmysqlclient REQUIRED)

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
	.
	)

file(GLOB_RECURSE ALL_FILES *.h *.cpp)
source_group_by_dir(ALL_FILES)

add_library(waterside_mysql STATIC ${ALL_FILES})

target_link_libraries(waterside_mysql libmysqlclient::libmysqlclient waterside_core)
