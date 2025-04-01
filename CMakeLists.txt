cmake_minimum_required(VERSION 3.12)
project(vypr VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(VYPR_SOURCES
    src/main.cpp
    src/token.cpp
    src/lexer.cpp
    src/parser.cpp
    src/semantic_analyzer.cpp
    src/ir_generator.cpp
    src/code_generator.cpp
    src/compiler.cpp
)

# Header files (for dependency tracking)
set(VYPR_HEADERS
    include/token.h
    include/lexer.h
    include/ast.h
    include/parser.h
    include/semantic_analyzer.h
    include/ir_generator.h
    include/code_generator.h
    include/compiler.h
    include/exceptions.h
)

# Add executable
add_executable(vypr ${VYPR_SOURCES} ${VYPR_HEADERS})

# Install target
install(TARGETS vypr DESTINATION bin)

# Setting output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add compile warnings
if(MSVC)
    target_compile_options(vypr PRIVATE /W4)
else()
    target_compile_options(vypr PRIVATE -Wall -Wextra -Wpedantic)
endif() 