# Minimum CMake version required
cmake_minimum_required(VERSION 3.15)

# Prefix Paths
list(APPEND CMAKE_PREFIX_PATH "Z:/Libraries/SDL2-2.32.0/cmake")
list(APPEND CMAKE_PREFIX_PATH "Z:/Libraries/SDL2-2.32.0")

# Project Name
project(DigiviceSim CXX)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2 Library
find_package(SDL2 REQUIRED)

# --- Debug Messages ---
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
message(STATUS "SDL2 Include Dirs: ${SDL2_INCLUDE_DIRS}")
message(STATUS "SDL2 Libraries: ${SDL2_LIBRARIES}")

# --- Add Your Source Files ---
add_executable(${PROJECT_NAME}
    main.cpp
    src/core/Game.cpp
    src/core/Game_fade.cpp
    src/core/InputManager.cpp
    src/core/AnimationManager.cpp
    src/Utils/AnimationUtils.cpp
    src/ui/TextRenderer.cpp
    src/platform/pc/pc_display.cpp
    src/core/AssetManager.cpp
    src/graphics/Animation.cpp 
    src/graphics/Animator.cpp
    src/states/AdventureState.cpp
    src/states/MenuState.cpp
    src/states/TransitionState.cpp
    src/states/PartnerSelectState.cpp
    src/states/ProgressState.cpp
    src/states/BattleState.cpp
    src/States/MapSystemState.cpp
)

# --- Include Directories --- 
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_SOURCE_DIR}/include"           # Project includes (This covers Z:/DigiviceRefactor/include/ AND its subdirectories like vendor/)
    # "${CMAKE_SOURCE_DIR}/vendor"          # <<< --- REMOVED THIS LINE --- >>> (because json.hpp is under include/vendor)
    ${SDL2_INCLUDE_DIRS}                    # SDL2 includes
    "Z:/Libraries/SDL2_image-2.8.6/include" # Manual SDL_image include path
)

# --- Link Libraries ---
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${SDL2_LIBRARIES}                                       # Link SDL2 found by find_package
    "Z:/Libraries/SDL2_image-2.8.6/lib/x64/SDL2_image.lib"  # Manual link to SDL2_image
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