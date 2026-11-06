cmake_minimum_required(VERSION 3.22)

project(nes)

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
	nes.c
	nes.h
	controller.c
	controller.h
	debug_tools.c
	debug_tools.h
	MMC1.c
	MMC1.h
)

set(SOURCES_UI
	ui.c
)

set(SOURCES_PATT
	print_pattern_table.c
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
	nes.c
	nes.h
	controller.c
	controller.h
	debug_tools.c
	debug_tools.h
	MMC1.c
	MMC1.h
)

add_compile_options(-g -O0 -gdwarf-2)
add_link_options(-g -gdwarf-2)

add_subdirectory(SDL)
#file(COPY_FILE ${CMAKE_BINARY_DIR}/SDL/SDL3.dll ${CMAKE_BINARY_DIR}/SDL3.dll ONLY_IF_DIFFERENT)

# Zeus Engine Library
set(SOURCES_ZEUS
	zeus.c
	zeus.h
	fpoint.c
	fpoint.h
	utils.h
	utils.c
)
add_library(zeus STATIC ${SOURCES_ZEUS})
target_include_directories(zeus PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(zeus PUBLIC SDL3::SDL3)

# Executables
add_executable(nes ${SOURCES})
target_link_libraries(nes PRIVATE zeus)
target_include_directories(nes PRIVATE ${CMAKE_CURRENT_DIR})

add_executable(ui ${SOURCES_UI})
target_link_libraries(ui PRIVATE zeus)
target_include_directories(ui PRIVATE ${CMAKE_CURRENT_DIR})

add_executable(patt ${SOURCES_PATT})
target_link_libraries(patt PRIVATE zeus)
target_include_directories(patt PRIVATE ${CMAKE_CURRENT_DIR})

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
		nes.c
		nes.h
		controller.c
		controller.h
		MMC1.c
		MMC1.h
		debug_tools.c
		debug_tools.h
		arithmetic_tests.cpp
		shift_tests.cpp
		tests.h
	)
	add_subdirectory(googletest)
	add_executable(cpu_tests ${SOURCES_TESTS})
	target_link_libraries(cpu_tests PRIVATE zeus gtest)
	target_include_directories(cpu_tests PRIVATE ${CMAKE_CURRENT_DIR} googletest/include)
	#add_subdirectory(tests)
endif()
