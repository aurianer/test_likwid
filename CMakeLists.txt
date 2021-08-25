cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(small_test CXX)

find_package(HPX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

#add_executable(main main_with_simd.cpp)
add_executable(main main_stable_flops.cpp)
#add_executable(main main_without_simd.cpp)

include(HPX_SetupLikwid)
target_link_libraries(main PUBLIC Likwid::likwid)
target_link_libraries(main PUBLIC HPX::hpx)
target_compile_definitions(main PRIVATE LIKWID_PERFMON)
target_compile_options(main PUBLIC "-march=native" "-fno-tree-vectorize")
