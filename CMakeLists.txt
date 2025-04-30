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
    src/core/Game.cpp # Assuming location src/Game.cpp
    src/states/AdventureState.cpp
    src/platform/pc/pc_display.cpp
    src/graphics/Animation.cpp
    src/core/AssetManager.cpp
)

# --- Include Directories VIA COMPILER OPTIONS ---
target_compile_options(${PROJECT_NAME} PRIVATE
    "/I${CMAKE_SOURCE_DIR}/include"
    # "/I${CMAKE_SOURCE_DIR}/assets" # Uncomment if needed
    "/I${SDL2_INCLUDE_DIRS}"
    "/IZ:/Libraries/SDL2_image-2.8.6/include" # Manual SDL_image include path
)

# --- Link Libraries ---
target_link_libraries(${PROJECT_NAME} PUBLIC # Still use PUBLIC for SDL2
    ${SDL2_LIBRARIES}                   # SDL2 Libraries (From find_package)
    "Z:/Libraries/SDL2_image-2.8.6/lib/x64/SDL2_image.lib" # Manual path to SDL2_image .lib
)


# <<< --- ADDED ASSET COPYING BLOCK --- >>>
# --- Copy Assets to Output Directory Post-Build ---
set(ASSET_SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets")

# Use $<TARGET_FILE_DIR:...> to get the executable's output directory
# Append /assets to specify the destination subfolder
set(ASSET_DESTINATION_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets")

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSET_SOURCE_DIR}" "${ASSET_DESTINATION_DIR}"
    COMMENT "Copying assets from ${ASSET_SOURCE_DIR} to ${ASSET_DESTINATION_DIR}..."
    VERBATIM
)
# --- End Asset Copying Block ---


# --- Optional: Add build options for debugging (Unchanged) ---
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