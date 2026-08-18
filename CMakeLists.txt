cmake_minimum_required(VERSION 3.22)

project(cpu)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

set(SOURCES 
	main.c
	cpu.c
	cpu.h
)

add_executable(cpu ${SOURCES})
target_include_directories(cpu PUBLIC ${CMAKE_CURRENT_DIR})

if(TESTS)
	add_subdirectory(tests)
endif()
