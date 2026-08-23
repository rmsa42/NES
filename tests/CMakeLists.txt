cmake_minimum_required(VERSION 3.22)

project(cpu_tests)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(BUILD_GMOCK 0)

set(SOURCES 
	../cpu.c
	../cpu.h
	arithmetic_tests.cpp
)

add_subdirectory(googletest)

add_executable(cpu_tests ${SOURCES})
target_link_libraries(cpu_tests gtest)
target_include_directories(cpu_tests PUBLIC ${CMAKE_SOURCE_DIR} googletest/include)

