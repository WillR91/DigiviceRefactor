### State Management System

The application uses a finite state machine pattern where each game mode is represented by a distinct state:

```
Game (State Manager)
├── MenuState           # Main menu navigation and submenus
├── AdventureState      # Partner interaction and world exploration
├── BattleState         # Combat encounters with turn-based mechanics
├── PartnerSelectState  # Digimon partner selection interface
├── SettingsState       # Configuration options (controls, display, audio)
├── PlayerTestState     # Debug state for testing player Digimon
├── EnemyTestState      # Debug state for testing enemy Digimon
├── ProgressState       # Save/load and progress management
├── MapSystemState      # Map selection and navigation
└── TransitionState     # Visual transitions between states
```

**Key Components:**
- `Game` class - Central state manager with fade transition system
- `GameState` (base class) - Defines common state interface with lifecycle methods
- State-specific implementations handle their own rendering, input, and logic
- Fade transition system for smooth state changes