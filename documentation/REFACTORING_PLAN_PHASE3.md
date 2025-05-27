# Phase 3: Feature Enhancement & Polish (4-5 weeks)

## 3.1 Advanced Animation System

### Current Limitations
- Basic frame-based animations only
- No animation blending or transitions
- Limited event system integration
- No procedural or physics-based animation

### Enhanced Animation Framework

#### Animation State Machine
```cpp
// src/graphics/AnimationStateMachine.h
class AnimationStateMachine {
public:
    enum class TransitionType {
        Immediate,
        Crossfade,
        AfterCompletion
    };
    
    struct AnimationState {
        std::string name;
        const AnimationData* animationData;
        bool loops;
        float speed = 1.0f;
        std::function<void()> onComplete;
        std::function<void(int)> onFrameEvent; // Frame-specific events
    };
    
    struct Transition {
        std::string fromState;
        std::string toState;
        TransitionType type;
        float duration; // For crossfade
        std::function<bool()> condition;
    };
    
private:
    std::unordered_map<std::string, AnimationState> states_;
    std::vector<Transition> transitions_;
    std::string currentState_;
    std::string nextState_;
    float transitionProgress_ = 0.0f;
    bool inTransition_ = false;
    
public:
    void addState(const std::string& name, const AnimationState& state);
    void addTransition(const Transition& transition);
    void setState(const std::string& name);
    void update(float deltaTime);
    
    // Rendering support for blended animations
    void getCurrentFrame(SDL_Texture*& texture, SDL_Rect& srcRect, float& alpha) const;
    void getBlendedFrame(SDL_Texture*& tex1, SDL_Rect& rect1, float& alpha1,
                        SDL_Texture*& tex2, SDL_Rect& rect2, float& alpha2) const;
};
```

#### Enhanced Animator Class
```cpp
// src/graphics/EnhancedAnimator.h
class EnhancedAnimator {
    AnimationStateMachine stateMachine_;
    std::vector<std::pair<std::string, float>> animationQueue_; // For chaining
    
public:
    // State machine interface
    void setupStateMachine(const std::string& configFile);
    void triggerTransition(const std::string& trigger);
    
    // Animation events
    void setFrameEventCallback(std::function<void(const std::string&, int)> callback);
    void setCompletionCallback(std::function<void(const std::string&)> callback);
    
    // Advanced playback control
    void setSpeed(float speed);
    void pause();
    void resume();
    void queueAnimation(const std::string& name, float delay = 0.0f);
    
    // Blended rendering
    void renderBlended(PCDisplay& display, const SDL_Rect& destRect);
};

// Usage in Digimon entities
class DigimonEntity {
    EnhancedAnimator animator_;
    
public:
    void initialize() {
        animator_.setupStateMachine("animations/agumon_states.json");
        
        // Setup behavior-driven transitions
        animator_.setFrameEventCallback([this](const std::string& anim, int frame) {
            if (anim == "attack" && frame == 5) {
                // Trigger damage at frame 5 of attack animation
                dealDamage();
            }
        });
    }
    
    void startBattle() {
        animator_.triggerTransition("enter_battle");
    }
};
```

#### JSON Configuration for Animation States
```json
// assets/animations/agumon_states.json
{
  "states": {
    "idle": {
      "animation": "agumon_idle",
      "loops": true,
      "speed": 1.0
    },
    "walking": {
      "animation": "agumon_walk", 
      "loops": true,
      "speed": 1.5
    },
    "attack": {
      "animation": "agumon_attack",
      "loops": false,
      "frameEvents": {
        "5": "damage_frame",
        "10": "recovery_start"
      }
    }
  },
  "transitions": [
    {
      "from": "idle",
      "to": "walking", 
      "trigger": "start_walking",
      "type": "crossfade",
      "duration": 0.2
    },
    {
      "from": "*",
      "to": "attack",
      "trigger": "attack_command",
      "type": "immediate"
    }
  ]
}
```

## 3.2 Advanced Input System

### Current Limitations
- Basic key mapping only
- No controller support
- Limited accessibility options
- No gesture recognition

### Enhanced Input Framework

#### Input Action System
```cpp
// src/input/InputActionSystem.h
class InputActionSystem {
public:
    enum class InputType {
        Keyboard,
        Gamepad,
        Touch,
        Gesture
    };
    
    struct InputBinding {
        InputType type;
        union {
            SDL_Scancode keycode;
            struct { int controller; int button; } gamepad;
            struct { float x, y, float radius; } touchArea;
        };
        float deadzone = 0.1f; // For analog inputs
    };
    
    struct InputAction {
        std::string name;
        std::vector<InputBinding> bindings;
        bool isAnalog = false;
        std::function<void(float)> callback; // Value 0-1 for digital, -1 to 1 for analog
    };
    
private:
    std::unordered_map<std::string, InputAction> actions_;
    std::vector<SDL_GameController*> controllers_;
    
public:
    void loadBindings(const std::string& configFile);
    void addAction(const InputAction& action);
    void bindKey(const std::string& action, SDL_Scancode key);
    void bindGamepadButton(const std::string& action, int controller, int button);
    
    // Context system for different input modes
    void pushContext(const std::string& context);
    void popContext();
    
    void update();
    void handleEvent(const SDL_Event& event);
};
```

#### Controller Support
```cpp
// src/input/ControllerManager.h
class ControllerManager {
    std::vector<std::unique_ptr<Controller>> controllers_;
    
public:
    struct Controller {
        SDL_GameController* controller;
        int instanceId;
        std::string name;
        bool isConnected;
        
        // State tracking
        std::array<bool, SDL_CONTROLLER_BUTTON_MAX> buttonStates;
        std::array<bool, SDL_CONTROLLER_BUTTON_MAX> buttonPressed;
        std::array<int16_t, SDL_CONTROLLER_AXIS_MAX> axisStates;
    };
    
    void initialize();
    void update();
    void onControllerAdded(int deviceIndex);
    void onControllerRemoved(int instanceId);
    
    Controller* getController(int index) const;
    int getControllerCount() const;
};

// Enhanced input configuration
// config/input_bindings.json
{
  "contexts": {
    "menu": {
      "navigate_up": [
        {"type": "keyboard", "key": "up"},
        {"type": "gamepad", "controller": -1, "button": "dpad_up"}
      ],
      "confirm": [
        {"type": "keyboard", "key": "return"},
        {"type": "gamepad", "controller": -1, "button": "a"}
      ]
    },
    "adventure": {
      "move_horizontal": [
        {"type": "keyboard", "keys": ["left", "right"], "analog": true},
        {"type": "gamepad", "controller": -1, "axis": "leftx", "analog": true}
      ]
    }
  }
}
```

## 3.3 Advanced Audio System

### Current State
Basic audio support through SDL2, limited functionality.

### Enhanced Audio Framework

#### Audio Manager with 3D Positioning
```cpp
// src/audio/AudioManager.h
class AudioManager {
public:
    struct Sound {
        Mix_Chunk* chunk;
        std::string name;
        float defaultVolume = 1.0f;
        bool is3D = false;
    };
    
    struct Music {
        Mix_Music* music;
        std::string name;
        bool loops = true;
    };
    
    struct AudioSource {
        int channel = -1;
        Vec2 position;
        float volume = 1.0f;
        float pitch = 1.0f;
        bool isPlaying = false;
    };
    
private:
    std::unordered_map<std::string, Sound> sounds_;
    std::unordered_map<std::string, Music> music_;
    std::vector<AudioSource> sources_;
    
    Vec2 listenerPosition_;
    float masterVolume_ = 1.0f;
    float musicVolume_ = 1.0f;
    float sfxVolume_ = 1.0f;
    
public:
    bool initialize();
    void shutdown();
    
    // Asset loading
    bool loadSound(const std::string& name, const std::string& path);
    bool loadMusic(const std::string& name, const std::string& path);
    
    // Playback control
    int playSound(const std::string& name, float volume = 1.0f);
    int playSoundAt(const std::string& name, const Vec2& position, float volume = 1.0f);
    void stopSound(int sourceId);
    void stopAllSounds();
    
    void playMusic(const std::string& name, bool fadeIn = false);
    void stopMusic(bool fadeOut = false);
    void crossfadeMusic(const std::string& newMusic, float duration = 2.0f);
    
    // 3D audio
    void setListenerPosition(const Vec2& position);
    void updateSourcePosition(int sourceId, const Vec2& position);
    
    // Volume control
    void setMasterVolume(float volume);
    void setMusicVolume(float volume);
    void setSFXVolume(float volume);
    
    void update(float deltaTime);
};
```

#### Adaptive Music System
```cpp
// src/audio/AdaptiveMusicManager.h
class AdaptiveMusicManager {
    struct MusicLayer {
        std::string name;
        Mix_Music* music;
        float volume = 0.0f;
        float targetVolume = 0.0f;
        bool isPlaying = false;
    };
    
    struct MusicState {
        std::string name;
        std::vector<std::string> layers;
        std::unordered_map<std::string, float> layerVolumes;
    };
    
    std::unordered_map<std::string, MusicLayer> layers_;
    std::unordered_map<std::string, MusicState> states_;
    std::string currentState_;
    float transitionSpeed_ = 2.0f;
    
public:
    void addLayer(const std::string& name, const std::string& path);
    void addState(const std::string& name, const MusicState& state);
    void transitionToState(const std::string& state, float duration = 2.0f);
    void setLayerVolume(const std::string& layer, float volume);
    
    void update(float deltaTime);
};

// Usage for dynamic battle music
// In BattleState
void BattleState::onHealthChanged(float playerHealth, float enemyHealth) {
    if (playerHealth < 0.3f) {
        musicManager.transitionToState("battle_desperate");
    } else if (enemyHealth < 0.3f) {
        musicManager.transitionToState("battle_winning");
    } else {
        musicManager.transitionToState("battle_normal");
    }
}
```

## 3.4 Save System & Data Persistence

### Current State
Basic PlayerData structure, limited save functionality.

### Enhanced Save System

#### Versioned Save Format
```cpp
// src/save/SaveManager.h
class SaveManager {
public:
    static constexpr int CURRENT_SAVE_VERSION = 3;
    
    struct SaveHeader {
        uint32_t magic = 0x44495649; // "DIVI"
        uint32_t version = CURRENT_SAVE_VERSION;
        uint64_t timestamp;
        uint32_t checksum;
        uint32_t dataSize;
    };
    
    struct SaveData {
        PlayerData playerData;
        DigimonData partnerData;
        ProgressData progressData;
        SettingsData settingsData;
        
        // Extensible metadata
        std::unordered_map<std::string, json> customData;
    };
    
private:
    std::string saveDirectory_;
    
public:
    bool saveGame(const SaveData& data, int slot = 0);
    bool loadGame(SaveData& data, int slot = 0);
    bool hasSaveFile(int slot = 0) const;
    SaveHeader getSaveHeader(int slot = 0) const;
    
    // Backup and migration
    bool createBackup(int slot);
    bool migrateSave(int fromVersion, int toVersion, json& saveJson);
    
    // Auto-save functionality
    void enableAutoSave(float intervalMinutes = 5.0f);
    void disableAutoSave();
    void performAutoSave();
};
```

#### Cloud Save Integration (Future)
```cpp
// src/save/CloudSaveManager.h
class CloudSaveManager {
public:
    enum class SyncStatus {
        NotConnected,
        Syncing,
        UpToDate,
        Conflict,
        Error
    };
    
    struct CloudSave {
        std::string saveId;
        uint64_t timestamp;
        std::string deviceId;
        size_t dataSize;
    };
    
    virtual bool uploadSave(const SaveData& data) = 0;
    virtual bool downloadSave(SaveData& data) = 0;
    virtual std::vector<CloudSave> listCloudSaves() = 0;
    virtual SyncStatus getSyncStatus() const = 0;
    
    void setConflictResolution(std::function<SaveData(const SaveData&, const SaveData&)> resolver);
};
```

## 3.5 Localization System

### Multi-Language Support
```cpp
// src/localization/LocalizationManager.h
class LocalizationManager {
public:
    enum class Language {
        English,
        Japanese,
        Spanish,
        French,
        German,
        Italian,
        Portuguese,
        Korean,
        ChineseSimplified,
        ChineseTraditional
    };
    
private:
    Language currentLanguage_ = Language::English;
    std::unordered_map<std::string, std::unordered_map<Language, std::string>> strings_;
    std::unordered_map<Language, std::string> fontPaths_;
    
public:
    bool loadLanguage(Language language, const std::string& path);
    void setLanguage(Language language);
    Language getCurrentLanguage() const { return currentLanguage_; }
    
    std::string getString(const std::string& key) const;
    std::string getFormattedString(const std::string& key, const std::vector<std::string>& args) const;
    
    // Font management for different languages
    std::string getFontPath(Language language) const;
    bool hasFont(Language language) const;
};

// Usage throughout UI
// Instead of hardcoded strings
textRenderer->drawText(renderer, "Health:", x, y, scale);

// Use localized strings
auto& loc = LocalizationManager::getInstance();
textRenderer->drawText(renderer, loc.getString("ui.health_label"), x, y, scale);
```

## Implementation Timeline

### Week 1-2: Animation System
- [ ] Implement AnimationStateMachine
- [ ] Enhanced Animator with blending
- [ ] JSON configuration system
- [ ] Frame event system

### Week 3: Input & Audio Systems  
- [ ] Advanced input action system
- [ ] Controller support implementation
- [ ] Enhanced AudioManager with 3D positioning
- [ ] Adaptive music system

### Week 4: Save System
- [ ] Versioned save format
- [ ] Migration system for old saves
- [ ] Auto-save functionality
- [ ] Backup and recovery

### Week 5: Localization & Polish
- [ ] Localization framework
- [ ] Multi-language text support
- [ ] Font management for different languages
- [ ] UI/UX improvements based on testing feedback

## Success Metrics

1. **Animation**: Smooth 60fps with complex animation blending
2. **Input**: Full controller support with customizable bindings
3. **Audio**: Immersive 3D audio with adaptive music
4. **Saves**: 100% save/load reliability with migration support
5. **Localization**: Support for at least 3 languages

## Risk Mitigation

- Implement features incrementally with feature flags
- Maintain backward compatibility with existing content
- Comprehensive testing for save/load functionality
- Performance testing for animation blending
- User testing for input responsiveness
