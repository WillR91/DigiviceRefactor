#ifndef MAP_SYSTEM_STATE_H
#define MAP_SYSTEM_STATE_H

#include "states/GameState.h" // Corrected: Include GameState.h from states directory
#include "Core/MapData.h"
#include <vector>
#include <string>

// Forward declarations if needed

namespace Digivice {

    class MapSystemState : public GameState { // Corrected: Inherit from GameState
    public:
        // Enum to manage the internal views of the map system
        enum class MapView {
            CONTINENT_SELECTION,
            NODE_SELECTION,
            NODE_DETAIL
        };

        MapSystemState(Game* game); // Uncommented Game* game parameter
        ~MapSystemState() override;

        void enter() override;
        void exit() override;
        // Corrected handle_input signature to match GameState (assuming it's like AdventureState/MenuState)
        void handle_input(InputManager& inputManager, PlayerData* playerData) override;
        void update(float delta_time, PlayerData* playerData) override;
        void render(PCDisplay& display) override;
        StateType getType() const override;

    private:
        MapView currentView_;
        
        // Data for the map system
        std::vector<ContinentData> continents_;
        int currentContinentIndex_;
        int currentNodeIndex_;

        // Helper methods for each view (to be implemented later)
        void handle_input_continent_selection(float dt);
        void handle_input_node_selection(float dt);
        void handle_input_node_detail(float dt);

        void update_continent_selection(float dt);
        void update_node_selection(float dt);
        void update_node_detail(float dt);

        void render_continent_selection();
        void render_node_selection();
        void render_node_detail();

        void load_map_data(); // For initial hardcoded data loading
    };

} // namespace Digivice

#endif // MAP_SYSTEM_STATE_H
