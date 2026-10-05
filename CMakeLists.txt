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
	ppu.c
	ppu.h
	rom.c
	rom.h
	bus.c
	bus.h
	debug_tools.c
	debug_tools.h
	sdl_utils.c
	sdl_utils.h
	fpoint.c
	fpoint.h
)


add_subdirectory(SDL)
#file(COPY_FILE ${CMAKE_BINARY_DIR}/SDL/SDL3.dll ${CMAKE_BINARY_DIR}/SDL3.dll ONLY_IF_DIFFERENT)

add_executable(cpu ${SOURCES})
target_link_libraries(cpu PRIVATE SDL3::SDL3)
target_include_directories(cpu PRIVATE ${CMAKE_CURRENT_DIR})

if(TESTS)
	set(BUILD_GMOCK 0)
	set(SOURCES_TESTS
		cpu.c
		cpu.h
		apu.c
		apu.h
		ppu.c
		ppu.h
		rom.c
		rom.h
		bus.c
		bus.h
		debug_tools.c
		debug_tools.h
		sdl_utils.c
		sdl_utils.h
		fpoint.c
		fpoint.h
		arithmetic_tests.cpp
		shift_tests.cpp
		tests.h
	)
	add_subdirectory(googletest)
	add_executable(cpu_tests ${SOURCES_TESTS})
	target_link_libraries(cpu_tests PRIVATE SDL3::SDL3 gtest)
	target_include_directories(cpu_tests PRIVATE ${CMAKE_CURRENT_DIR} googletest/include)
	#add_subdirectory(tests)
endif()
