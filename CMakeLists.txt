# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project Name
project(DigiviceSim CXX) # CXX means C++

# Set C++ Standard (e.g., C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Find SDL2 Library ---
# IMPORTANT: Ensure you provide the path hint during CMake configuration, e.g., by setting
# SDL2_DIR=Z:/Libraries/SDL2-2.32.0/cmake in VS Code settings or via -D command line flag.
find_package(SDL2 REQUIRED)

# --- Debug Messages ---
message(STATUS "SDL2 Include Dirs (from find_package): ${SDL2_INCLUDE_DIRS}") # Should be Z:/Libraries/SDL2-2.32.0/include
message(STATUS "SDL2 Libraries Found (targets): ${SDL2_LIBRARIES}")
# -------------------------------------------

# --- Add Your Source Files ---
add_executable(${PROJECT_NAME}
    main.cpp
    src/Game.cpp
    src/AdventureState.cpp
    src/platform/pc/pc_display.cpp
    src/Animation.cpp
)

# --- Include Directories ---
# Add Your project's own include directories first
target_include_directories(${PROJECT_NAME} PUBLIC
    include     # For Game.h, AdventureState.h etc.
    assets      # For asset headers
)

# *** FIX: Explicitly add SDL2 include directory (HARDCODED PATH TEST) ***
# Using the literal path string instead of the variable ${SDL2_INCLUDE_DIRS}
# to see if this forces it into the standard /I list.
target_include_directories(${PROJECT_NAME} PUBLIC "Z:/Libraries/SDL2-2.32.0/include")
# ************************************************************************

# --- Link Libraries (Using Imported Targets) ---
target_link_libraries(${PROJECT_NAME} PUBLIC
    SDL2::SDL2
    SDL2::SDL2main
)

# --- Optional: Add build options for debugging ---
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR NOT CMAKE_BUILD_TYPE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /Zi)
        target_link_options(${PROJECT_NAME} PRIVATE /DEBUG)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -g)
    endif()
    message(STATUS "Debug build enabled")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE NDEBUG)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /O2)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -O3)
    endif()
    message(STATUS "Release build enabled")
endif()

# --- End of CMakeLists.txt ---