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
add_executable(DEMO1 main.cpp)
add_custom_command(TARGET DEMO1 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/res_demo1"
        "${CMAKE_CURRENT_BINARY_DIR}/res_demo1"
)

target_link_libraries(DEMO1 graphicengine)
graphicengine_setup(DEMO1)