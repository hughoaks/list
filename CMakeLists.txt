cmake_minimum_required(VERSION 3.14)
project(VerilogDatapathGenerator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/config.cpp
    src/signal.cpp
    src/operation.cpp
    src/control_block.cpp
    src/netlist_generator.cpp
    src/verilog_emitter.cpp
)

# Executable
add_executable(verilog_gen ${SOURCES})

# Installation
install(TARGETS verilog_gen DESTINATION bin)
