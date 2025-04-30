# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Prefix Paths (Only need SDL2 now)
list(APPEND CMAKE_PREFIX_PATH "Z:/Libraries/SDL2-2.32.0/cmake")
list(APPEND CMAKE_PREFIX_PATH "Z:/Libraries/SDL2-2.32.0")

# Project Name
project(DigiviceSim CXX)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Find ONLY SDL2 Library ---
find_package(SDL2 REQUIRED)

# --- Debug Messages ---
message(STATUS "CMAKE_PREFIX_PATH set to: ${CMAKE_PREFIX_PATH}")
message(STATUS "SDL2 Include Dirs Found (Used by find_package): ${SDL2_INCLUDE_DIRS}")
message(STATUS "SDL2 Libraries Found (Used by find_package): ${SDL2_LIBRARIES}")
# -------------------------------------------

# --- Add Your Source Files ---
add_executable(${PROJECT_NAME}
    main.cpp
    src/Game.cpp
    src/states/AdventureState.cpp
    src/platform/pc/pc_display.cpp
    src/graphics/Animation.cpp
    src/core/AssetManager.cpp
)

# --- Include Directories VIA COMPILER OPTIONS ---
# Use target_compile_options to add /I flags directly for MSVC
# This bypasses potential issues with target_include_directories and VS generator
target_compile_options(${PROJECT_NAME} PRIVATE
    # Use MSVC syntax /I"path" (Quotes needed if path has spaces, good practice anyway)
    "/I${CMAKE_SOURCE_DIR}/include"           # Your Project's Include Dir
    # "/I${CMAKE_SOURCE_DIR}/assets"          # Uncomment if needed for asset headers
    "/I${SDL2_INCLUDE_DIRS}"                  # SDL2 Headers (CMake variable should be fine here)
    "/IZ:/Libraries/SDL2_image-2.8.6/include" # <<< Force SDL2_image include path >>>
)
# --- REMOVED target_include_directories block ---


# --- Link Libraries ---
# Link the libraries themselves.
target_link_libraries(${PROJECT_NAME} PUBLIC # Still use PUBLIC for SDL2 if your headers include SDL.h
    ${SDL2_LIBRARIES}                   # SDL2 Libraries (From find_package)

    # <<< ADDED Manual path to SDL2_image .lib file >>>
    # Link against the specific .lib file for your architecture (x64)
    "Z:/Libraries/SDL2_image-2.8.6/lib/x64/SDL2_image.lib" # Keep this from previous step
)

# --- Optional: Add build options for debugging (Unchanged) ---
# ... (rest of file) ...

# --- End of CMakeLists.txt ---