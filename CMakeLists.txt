cmake_minimum_required(VERSION 3.15)

    project(meowlang_cpp LANGUAGES CXX)
        
    # Set CXX standard
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
        
    # Find vcpkg packages if available
    find_package(PkgConfig QUIET)
        
    # Include directories
    include_directories(include)
        
    # Add executable
    add_executable(meowlang_cpp 
            src/main.cpp
    )
        
    # Compiler-specific options
    if(MSVC)
        target_compile_options(meowlang_cpp PRIVATE /W4)
    else()
        target_compile_options(meowlang_cpp PRIVATE -Wall -Wextra -Wpedantic)
    endif()
        
    # Debug configuration
    set(CMAKE_BUILD_TYPE Debug)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(meowlang_cpp PRIVATE DEBUG)
        if(NOT MSVC)
            target_compile_options(meowlang_cpp PRIVATE -g -O0)
        endif()
    endif()
        
    # Enable testing
    enable_testing()
        
    # Install target
    #install(TARGETS meowlang_cpp RUNTIME DESTINATION bin)
    