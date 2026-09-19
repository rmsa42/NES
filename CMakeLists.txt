cmake_minimum_required(VERSION 3.22)

project(cpu)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-parameter")

set(SOURCES 
	main.c
	cpu.c
	cpu.h
	apu.c
	apu.h
	rom.c
	rom.h
)

add_subdirectory(SDL)
#file(COPY_FILE ${CMAKE_BINARY_DIR}/SDL/SDL3.dll ${CMAKE_BINARY_DIR}/SDL3.dll ONLY_IF_DIFFERENT)

add_executable(cpu ${SOURCES})
target_link_libraries(cpu PRIVATE SDL3::SDL3)
target_include_directories(cpu PRIVATE ${CMAKE_CURRENT_DIR})


if(TESTS)
	add_subdirectory(tests)
endif()
