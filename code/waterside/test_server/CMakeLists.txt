find_package(TBB REQUIRED)

include_directories(
	../waterside_core
	../waterside_core/logger
	../waterside_core/signal
	../waterside_core/id
	../waterside_core/object
	../waterside_core/time
	../waterside_core/network
	.
	)

add_executable (test_server "main.cpp")

target_link_libraries(test_server PRIVATE waterside_core)