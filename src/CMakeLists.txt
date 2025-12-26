add_subdirectory(engine)

add_executable(invariant main.cpp)
target_link_libraries(invariant PRIVATE engine)

set_target_properties(invariant PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)