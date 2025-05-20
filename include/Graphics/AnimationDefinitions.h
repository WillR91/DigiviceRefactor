// include/graphics/AnimationDefinitions.h
#ifndef ANIMATIONDEFINITIONS_H
#define ANIMATIONDEFINITIONS_H

#include <vector>
#include <SDL_stdinc.h> // For Uint32

namespace AnimDefs {    // --- Shared Animation Sequence Templates ---

    // Idle Animation (Assumes 2 frames)
    // Updated to use 1-based indices to match JSON frame names
    const std::vector<int> IDLE_INDICES = {1, 2};
    const std::vector<Uint32> IDLE_DURATIONS_MS = {800, 800}; // Durations in Milliseconds

    // Walk Animation (Assumes 4 frames, a simple back-and-forth)
    // Updated to use 1-based indices to match JSON frame names
    const std::vector<int> WALK_INDICES = {3, 4, 3, 4};
    const std::vector<Uint32> WALK_DURATIONS_MS = {300, 300, 300, 300}; // Durations in Milliseconds

    // Attack Animation (Example, if you have one defined like this)
    // const std::vector<int> ATTACK_INDICES = {1, 0, 3, 8}; // Example from AdventureState
    // const std::vector<Uint32> ATTACK_DURATIONS_MS = {200, 150, 150, 400}; // Example from AdventureState    // Kuwagamon Enemy Digimon animations
    // Updated to use 1-based indices to match JSON frame names
    const std::vector<int> KUWAGAMON_IDLE_INDICES = {1, 2};
    const std::vector<Uint32> KUWAGAMON_IDLE_DURATIONS_MS = {800, 800};
    
    // Updated to use 1-based indices to match JSON frame names
    const std::vector<int> KUWAGAMON_ATTACK_INDICES = {3, 4, 3};
    const std::vector<Uint32> KUWAGAMON_ATTACK_DURATIONS_MS = {200, 300, 250};

} // namespace AnimDefs

#endif // ANIMATIONDEFINITIONS_H