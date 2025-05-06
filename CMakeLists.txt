# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Prefix Paths (Only need SDL2 now)
# Consider removing these if find_package works without them, or keep if necessary
list(APPEND CMAKE_PREFIX_PATH "Z:/Libraries/SDL2-2.32.0/cmake")
list(APPEND CMAKE_PREFIX_PATH "Z:/Libraries/SDL2-2.32.0")

# Project Name
project(DigiviceSim CXX)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Find SDL2 and SDL2_image Libraries ---
# Find SDL2 (Required)
find_package(SDL2 REQUIRED)
# Find SDL2_image (Optional, adjust as needed if REQUIRED)
# You might need to set SDL2_image_DIR or adjust CMAKE_PREFIX_PATH
# for find_package to find it automatically. If not, keep manual paths.
# find_package(SDL2_image) # Try uncommenting this if you set up SDL2_image's CMake config

# --- Debug Messages ---
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
message(STATUS "SDL2 Include Dirs: ${SDL2_INCLUDE_DIRS}")
message(STATUS "SDL2 Libraries: ${SDL2_LIBRARIES}")
# if(SDL2_image_FOUND)
#     message(STATUS "SDL2_image Include Dirs: ${SDL2_IMAGE_INCLUDE_DIRS}")
#     message(STATUS "SDL2_image Libraries: ${SDL2_IMAGE_LIBRARIES}")
# else()
#     message(WARNING "SDL2_image not found by find_package. Using manual paths.")
# endif()
# -------------------------------------------

# --- Add Your Source Files ---
add_executable(${PROJECT_NAME}
    main.cpp
    src/core/Game.cpp
    src/core/InputManager.cpp
    src/core/AnimationManager.cpp
    src/ui/TextRenderer.cpp
    # Add PlayerData source if you create one later
    src/platform/pc/pc_display.cpp
    src/core/AssetManager.cpp
    src/graphics/Animation.cpp
    src/graphics/Animator.cpp
    # States
    src/states/AdventureState.cpp
    src/states/MenuState.cpp
    src/states/TransitionState.cpp
    src/states/PartnerSelectState.cpp # Add PartnerSelectState source file
)

# <<< --- MODIFIED Include Directories --- >>>
# Use target_include_directories for better handling and portability
target_include_directories(${PROJECT_NAME} PUBLIC # Use PUBLIC so interfaces are visible if needed
    "${CMAKE_SOURCE_DIR}/include"           # Your project's include dir
    ${SDL2_INCLUDE_DIRS}                    # SDL2 include dir(s) (handles list correctly)
    "Z:/Libraries/SDL2_image-2.8.6/include" # Manual SDL_image include path (if find_package fails)
    # No need to add include/vendor separately if nlohmann/json.hpp is inside include/vendor
)
# --- END MODIFIED Include Directories --- >>>


# --- Link Libraries ---
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${SDL2_LIBRARIES}                       # SDL2 Libraries (From find_package)
    # ${SDL2_IMAGE_LIBRARIES}               # Use this if find_package(SDL2_image) works
    "Z:/Libraries/SDL2_image-2.8.6/lib/x64/SDL2_image.lib" # Manual path (if find_package fails)
)


# --- Copy Assets to Output Directory Post-Build ---
set(ASSET_SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets")
set(ASSET_DESTINATION_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets")
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSET_SOURCE_DIR}" "${ASSET_DESTINATION_DIR}"
    COMMENT "Copying assets from ${ASSET_SOURCE_DIR} to ${ASSET_DESTINATION_DIR}..."
    VERBATIM
)


# --- Optional: Add build options for debugging ---
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR NOT CMAKE_BUILD_TYPE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /Zi) # Keep compiler-specific debug info flag
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