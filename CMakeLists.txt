cmake_minimum_required(VERSION 3.28)

project(cpu)

set(SOURCES 
	main.c
	cpu.c
	cpu.h
)

add_executable(cpu ${SOURCES})
target_include_directories(cpu PUBLIC ${CMAKE_CURRENT_DIR})
