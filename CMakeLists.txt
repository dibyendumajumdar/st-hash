cmake_minimum_required(VERSION 3.16)

project(st_project C)

# Build library
add_library(st st.c)

target_include_directories(st PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Build executable
add_executable(test_st test_st.c)

target_link_libraries(test_st PRIVATE st)
