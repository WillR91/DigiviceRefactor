# DigiviceRefactor: Feature Documentation

## Core Game Features

### 1. Digimon Management System

#### Digimon Registry
- **Centralized database**: Single source for all Digimon definitions
- **Classification system**: 
  - `PlayerRookie`: Starting partner Digimon (8 available)
  - `PlayerChampion`: Evolution forms (not yet implemented)
  - `PlayerUnlockables`: Special Digimon (Veedramon, Wizardmon)
  - `StandardEnemy`: Regular battle opponents (60+ creatures)
  - `Boss`: Special powerful enemies
  - `NPC`: Non-combat characters

#### Player Digimon (Fully Implemented)
1. **Agumon** - Fire-type dragon rookie
2. **Gabumon** - Ice-type beast rookie  
3. **Biyomon** - Air-type bird rookie
4. **Gatomon** - Light-type cat champion
5. **Gomamon** - Water-type seal rookie
6. **Palmon** - Plant-type flower rookie
7. **Tentomon** - Electric-type insect rookie
8. **Patamon** - Wind-type mammal rookie

#### Unlockable Digimon
- **Veedramon** - Special dragon Digimon
- **Wizardmon** - Magical humanoid Digimon

### 2. Animation System

#### Animation Types
Each Digimon supports multiple animation states:
- **Idle**: Default standing animation (2 frames, 1000ms each)
- **Walk**: Movement animation (4 frames, 300ms each)
- **Attack**: Combat animation (timing varies)

#### Technical Implementation
- **Sprite sheets**: PNG textures with multiple character frames
- **JSON definitions**: Frame rectangles and timing data
- **Automatic loading**: AnimationManager loads all animation data at startup
- **Real-time playback**: Animator class handles frame timing and looping

#### File Structure
```
assets/sprites/player_digimon/
├── agumon.png          # Sprite sheet texture
├── agumon.json         # Animation frame definitions
├── gabumon.png
├── gabumon.json
└── ...
```

### 3. User Interface System

#### Text Rendering
- **Custom bitmap font**: Blue/white themed character atlas
- **JSON character mapping**: Flexible glyph-to-texture region mapping
- **Dynamic scaling**: Global and per-text size adjustment
- **Kerning support**: Adjustable character spacing
- **Fallback handling**: Uppercase substitution for missing characters

#### Menu Navigation
- **Controller support**: D-pad and button navigation
- **Keyboard controls**: Arrow keys and action buttons
- **State-based menus**: Hierarchical menu system with back navigation
- **Visual feedback**: Cursor highlighting and selection indicators

#### Settings Interface
- **Key binding editor**: Real-time key remapping for all game actions
- **Text scale adjustment**: Live preview of text size changes
- **Configuration persistence**: Settings saved to JSON config file

### 4. Game States

#### Main Menu (MenuState)
- **Primary navigation**: Adventure, Digimon, Debug options
- **Submenu support**: Nested menu structures
- **Background graphics**: Themed background textures

#### Partner Selection (PartnerSelectState)  
- **Digimon preview**: Real-time animation display
- **Navigation**: Cycle through available partners
- **Selection confirmation**: Choose starting partner

#### Adventure Mode (AdventureState)
- **Partner display**: Active partner Digimon animation
- **Background layers**: Multi-layer parallax backgrounds
- **State management**: Idle vs Walking animation states

#### Battle System (BattleState)
- **Turn-based combat**: Player vs enemy Digimon battles
- **Animation integration**: Combat animations for both participants
- **UI overlays**: Health bars and battle information

#### Debug Tools
- **PlayerTestState**: Test all player Digimon animations
  - LEFT/RIGHT: Change Digimon
  - UP/DOWN: Cycle animation types
  - ESC: Return to menu
- **EnemyTestState**: Test all enemy Digimon animations
  - Same controls as PlayerTestState
  - Access to 80+ enemy creatures

### 5. Input System

#### Default Controls
- **Navigation**: Arrow keys (UP/DOWN/LEFT/RIGHT)
- **Action**: Enter/Return key
- **Cancel**: Escape key
- **Menu**: M key
- **Screen toggle**: F key

#### Customization
- **Rebindable controls**: All actions can be remapped
- **Conflict prevention**: System keys protected from binding
- **Live configuration**: Changes apply immediately
- **Persistent settings**: Key bindings saved to config file

### 6. Asset Management

#### Automatic Loading
- **Texture loading**: All sprites loaded at game startup
- **Animation data**: JSON files parsed and cached
- **Error handling**: Missing assets logged with fallback behavior

#### Asset Organization
```
assets/
├── sprites/
│   ├── player_digimon/     # Player character animations
│   ├── enemy_digimon/      # Enemy character animations  
│   └── ui/                 # Interface elements
├── backgrounds/
│   ├── castlebackground*.png  # Adventure mode backgrounds
│   └── menu_*.png             # Menu backgrounds
├── ui/
│   ├── fonts/              # Text rendering atlases
│   ├── transition/         # Screen transition effects
│   └── mask/               # UI masking textures
└── definitions/
    └── digimon_definitions/   # Digimon data files
```

## Performance Features

### Efficient Rendering
- **Texture caching**: All assets loaded once at startup
- **Animation pooling**: Reusable animation instances
- **Optimized drawing**: Direct SDL2 texture blitting

### Memory Management
- **Smart pointers**: Automatic resource cleanup
- **Asset sharing**: Multiple objects share texture references
- **Configuration caching**: Settings loaded once and cached

## Debug and Development Tools

### Built-in Testing
- **Animation previews**: Real-time testing of all character animations
- **Registry inspection**: View all loaded Digimon definitions
- **Performance logging**: SDL2 logging system with categorized output

### Configuration System
- **JSON-based settings**: Human-readable configuration files
- **Runtime modification**: Many settings can be changed without restart
- **Default fallbacks**: Sensible defaults when config missing

This feature set represents the current stable implementation as of the latest build.
