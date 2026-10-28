# NES Emulator

This project is a NES Emulator written in C

## Table of Contents
1. [Project Structure](#project-structure)
2. [Dependencies](#dependencies)
3. [Compiling_Source_Code](#compiling-source-code)

## Project Structure
- `CMakeLists.txt`: Build tool
- `docs/`: My documents about this project
- `tests/`: Tests
- `palettes/`: NES palette files (.pal)
- `SDL`: SDL library submodule
- `googletest`: googletest framework submodule

## Dependencies
- SDL
- googletest

## Compiling Source Code
1. Clone the repository.
2. Create the build directory `mkdir build && cd build`
3. Run cmake `cmake .. && cmake --build .`.
4. Run the binary `./nes`

## Docs

Everything from NES docs and my docs

## My Docs

1. [Development](./Development.md)
2. [My-Goal](./Goal.md)

## NES Docs

6502 CPU Instruction Set [link](https://www.pagetable.com/c64ref/6502/?tab=2)
