\
#pragma once

#include <string>

struct Combatant {
    std::string name = "Unknown";
    std::string sprite_texture_id = "fallback_sprite"; // Asset ID for the texture
    int current_hp = 100;
    int max_hp = 100;
    int attack_power = 10;
    int defense_power = 5;
    // Add other stats as needed (speed, special attack, etc.)

    // Default constructor
    Combatant() = default;

    // Parameterized constructor (optional, but can be handy)
    Combatant(std::string n, std::string sprite_id, int hp, int atk, int def)
        : name(std::move(n)), sprite_texture_id(std::move(sprite_id)),
          current_hp(hp), max_hp(hp), attack_power(atk), defense_power(def) {}
};
