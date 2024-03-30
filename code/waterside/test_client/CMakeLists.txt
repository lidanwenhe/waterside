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
	../fbs
	.
	)

add_executable (test_client "main.cpp")

target_link_libraries(test_client PRIVATE waterside_core)