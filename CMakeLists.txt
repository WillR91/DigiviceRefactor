# File: CMakeLists.txt (Correct for Subdirectory Structure)

# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project Name
project(DigiviceSim CXX) # CXX means C++

# Set C++ Standard (e.g., C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Find SDL2 Library ---
# IMPORTANT: Ensure you provide the path hint during CMake configuration, e.g., by setting
# SDL2_DIR=Z:/Libraries/SDL2-2.32.0/cmake (or C:/...) in VS Code settings or via -D command line flag.
find_package(SDL2 REQUIRED)

# --- Debug Messages ---
message(STATUS "SDL2 Include Dirs (from find_package): ${SDL2_INCLUDE_DIRS}")
message(STATUS "SDL2 Libraries Found (targets): ${SDL2_LIBRARIES}")
# -------------------------------------------

# --- Add Your Source Files ---
add_executable(${PROJECT_NAME}
    main.cpp
    src/Game.cpp
    src/AdventureState.cpp
    # --- Ensure path to the CPP file is correct ---
    src/platform/pc/pc_display.cpp # Or wherever pc_display.cpp now lives
    # ---------------------------------------------
    src/Animation.cpp
)

# --- Include Directories ---
# Add only the top-level include and assets directories.
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"     # Base include directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"      # Assets directory
)

# --- Explicitly add SDL2 Include Directory (using variable) ---
# If C1083 for SDL.h returns, change this back to the hardcoded path:
# target_include_directories(${PROJECT_NAME} PUBLIC "Z:/Libraries/SDL2-2.32.0/include") # Or C:/... path
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})
# ---------------------------------------------------------------------

# --- Link Libraries (Using Imported Targets) ---
target_link_libraries(${PROJECT_NAME} PUBLIC
    SDL2::SDL2
    SDL2::SDL2main
)

# --- Optional: Add build options for debugging ---
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR NOT CMAKE_BUILD_TYPE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG)
    if(MSVC) # ... MSVC Debug flags ...
        target_compile_options(${PROJECT_NAME} PRIVATE /Zi)
        target_link_options(${PROJECT_NAME} PRIVATE /DEBUG)
    else() # ... Other Debug flags ...
        target_compile_options(${PROJECT_NAME} PRIVATE -g)
    endif()
    message(STATUS "Debug build enabled")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release") # ... Release flags ...
    target_compile_definitions(${PROJECT_NAME} PRIVATE NDEBUG)
    if(MSVC) # ... MSVC Release flags ...
        target_compile_options(${PROJECT_NAME} PRIVATE /O2)
    else() # ... Other Release flags ...
        target_compile_options(${PROJECT_NAME} PRIVATE -O3)
    endif()
    message(STATUS "Release build enabled")
endif()

# --- End of CMakeLists.txt ---