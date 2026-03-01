cmake_minimum_required(VERSION 3.31)
project(demos)
set(CMAKE_CXX_STANDARD 20)

# Include Graphics Engine
add_subdirectory(engine)
include_directories(engine/include)

# DEMO 0
set(DEMO0 "unit-tests")
add_executable(DEMO0 unit-tests.cpp)
target_link_libraries(DEMO0 graphicengine)
add_custom_command(TARGET DEMO0 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/res_demo0"
        "${CMAKE_CURRENT_BINARY_DIR}/res_demo0"
)

# DEMO 1
set(DEMO1 "sponza")
add_executable(DEMO1 sponza.cpp)
target_link_libraries(DEMO1 graphicengine)
add_custom_command(TARGET DEMO1 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/res_demo1"
        "${CMAKE_CURRENT_BINARY_DIR}/res_demo1"
)

# DEMO 2
set(DEMO2 "normals")
add_executable(DEMO2 normal_map.cpp)
target_link_libraries(DEMO2 graphicengine)
add_custom_command(TARGET DEMO2 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/res_demo2"
        "${CMAKE_CURRENT_BINARY_DIR}/res_demo2"
)

graphicengine_setup(DEMO0)
graphicengine_setup(DEMO1)
graphicengine_setup(DEMO2)
