# Phase 2: Architecture & Development Experience (3-4 weeks)

## 2.1 Testing Framework Implementation

### Current Gap
No automated testing for core game systems, making refactoring risky and regression-prone.

### Solution: Comprehensive Testing System

#### Unit Test Framework
```cpp
// tests/framework/GameTestFramework.h
class GameTestFramework {
    std::unique_ptr<Game> testGame_;
    std::unique_ptr<MockPCDisplay> mockDisplay_;
    std::unique_ptr<MockAssetManager> mockAssets_;
    
public:
    void setupTestGame();
    void teardownTestGame();
    void simulateFrames(int frameCount);
    void injectInput(GameAction action);
    
    // State testing helpers
    template<typename StateType>
    StateType* getCurrentState();
    
    void assertStateType(StateType expectedType);
    void assertNoErrors();
};

// Example test
TEST_CASE("AdventureState Background Rendering") {
    GameTestFramework framework;
    framework.setupTestGame();
    
    // Setup test state
    auto* adventureState = framework.pushState<AdventureState>("test_node");
    
    // Simulate rendering
    framework.simulateFrames(10);
    
    // Verify no errors occurred
    framework.assertNoErrors();
    
    // Verify textures were loaded
    REQUIRE(adventureState->hasBackgroundTextures());
}
```

#### Integration Tests
```cpp
// tests/integration/StateTransitionTests.cpp
TEST_CASE("Menu to Adventure State Transition") {
    GameTestFramework framework;
    framework.setupTestGame();
    
    // Start in menu
    framework.assertStateType(StateType::MENU);
    
    // Navigate to partner select
    framework.injectInput(GameAction::CONFIRM);
    framework.simulateFrames(1);
    framework.assertStateType(StateType::PARTNER_SELECT);
    
    // Select partner and transition to adventure
    framework.injectInput(GameAction::CONFIRM);
    framework.simulateFrames(30); // Allow transition time
    framework.assertStateType(StateType::ADVENTURE);
    
    framework.assertNoErrors();
}
```

#### Performance Tests
```cpp
// tests/performance/RenderingPerformanceTests.cpp
class PerformanceProfiler {
    std::unordered_map<std::string, std::vector<double>> timings_;
    
public:
    void startTimer(const std::string& name);
    void endTimer(const std::string& name);
    double getAverageTime(const std::string& name) const;
    void printReport() const;
};

TEST_CASE("Background Rendering Performance") {
    GameTestFramework framework;
    PerformanceProfiler profiler;
    
    auto* adventureState = framework.setupAdventureState();
    
    // Measure rendering performance
    for (int i = 0; i < 1000; ++i) {
        profiler.startTimer("background_render");
        adventureState->renderBackgrounds(framework.getDisplay());
        profiler.endTimer("background_render");
    }
    
    // Verify performance targets
    double avgTime = profiler.getAverageTime("background_render");
    REQUIRE(avgTime < 2.0); // Less than 2ms per background render
}
```

## 2.2 Development Tools & Debugging

### Debug Overlay System
```cpp
// src/debug/DebugOverlay.h
class DebugOverlay {
    bool enabled_ = false;
    std::vector<std::string> debugLines_;
    PerformanceMetrics metrics_;
    
public:
    void toggle() { enabled_ = !enabled_; }
    void addLine(const std::string& text);
    void addMetric(const std::string& name, double value);
    void render(PCDisplay& display, TextRenderer& textRenderer);
    
    // Game system inspection
    void inspectAssetManager(const AssetManager& assets);
    void inspectAnimationState(const Animator& animator);
    void inspectMemoryUsage();
};

// Integration in Game.cpp
class Game {
    std::unique_ptr<DebugOverlay> debugOverlay_;
    
    void handleInput() {
        // ... existing input handling ...
        
        if (inputManager_.isActionJustPressed(GameAction::DEBUG_TOGGLE)) {
            debugOverlay_->toggle();
        }
    }
    
    void render() {
        // ... existing rendering ...
        
        if (debugOverlay_->isEnabled()) {
            debugOverlay_->render(display_, *textRenderer_);
        }
    }
};
```

### Asset Validation Tools
```cpp
// tools/AssetValidator.h
class AssetValidator {
public:
    struct ValidationResult {
        bool isValid;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
    };
    
    static ValidationResult validateTexture(const std::string& path);
    static ValidationResult validateAnimation(const std::string& path);
    static ValidationResult validateDigimonDefinition(const std::string& path);
    static ValidationResult validateEnvironment(const std::string& environmentPath);
    
    // Batch validation
    static void validateAllAssets(const std::string& assetRoot);
};

// Command line tool
// tools/validate_assets.cpp
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: validate_assets <asset_directory>" << std::endl;
        return 1;
    }
    
    AssetValidator::validateAllAssets(argv[1]);
    return 0;
}
```

### Performance Profiling
```cpp
// src/profiling/Profiler.h
class Profiler {
    struct ProfileBlock {
        std::string name;
        std::chrono::high_resolution_clock::time_point startTime;
        double totalTime = 0.0;
        int callCount = 0;
    };
    
    std::unordered_map<std::string, ProfileBlock> blocks_;
    
public:
    class ScopedTimer {
        Profiler& profiler_;
        std::string name_;
        std::chrono::high_resolution_clock::time_point startTime_;
        
    public:
        ScopedTimer(Profiler& p, const std::string& name) 
            : profiler_(p), name_(name) 
            , startTime_(std::chrono::high_resolution_clock::now()) {}
        
        ~ScopedTimer() {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<double, std::milli>(endTime - startTime_);
            profiler_.addSample(name_, duration.count());
        }
    };
    
    void addSample(const std::string& name, double timeMs);
    void printReport() const;
    void reset();
};

// Usage macro
#define PROFILE_SCOPE(profiler, name) \
    Profiler::ScopedTimer timer(profiler, name)

// In performance-critical code
void AdventureState::render(PCDisplay& display) {
    PROFILE_SCOPE(Game::getProfiler(), "AdventureState::render");
    
    {
        PROFILE_SCOPE(Game::getProfiler(), "background_rendering");
        renderBackgrounds(display);
    }
    
    {
        PROFILE_SCOPE(Game::getProfiler(), "sprite_rendering");
        renderSprites(display);
    }
}
```

## 2.3 Build System & CI Improvements

### Enhanced CMake Configuration
```cmake
# CMakeLists.txt improvements
option(DIGIVICE_BUILD_TESTS "Build unit tests" ON)
option(DIGIVICE_BUILD_TOOLS "Build development tools" ON)
option(DIGIVICE_ENABLE_PROFILING "Enable profiling" OFF)
option(DIGIVICE_SANITIZERS "Enable address/memory sanitizers" OFF)

# Performance build settings
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(DigiviceSim PRIVATE NDEBUG)
    target_compile_options(DigiviceSim PRIVATE -O3 -march=native)
endif()

# Debug build settings  
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(DigiviceSim PRIVATE DEBUG_BUILD)
    if(DIGIVICE_SANITIZERS)
        target_compile_options(DigiviceSim PRIVATE -fsanitize=address)
        target_link_options(DigiviceSim PRIVATE -fsanitize=address)
    endif()
endif()

# Testing framework
if(DIGIVICE_BUILD_TESTS)
    find_package(Catch2 REQUIRED)
    add_subdirectory(tests)
endif()
```

### Continuous Integration Pipeline
```yaml
# .github/workflows/ci.yml
name: CI

on: [push, pull_request]

jobs:
  build-and-test:
    runs-on: windows-latest
    
    strategy:
      matrix:
        build_type: [Debug, Release]
        
    steps:
    - uses: actions/checkout@v3
    
    - name: Setup SDL2
      run: |
        # Install SDL2 dependencies
        
    - name: Configure CMake
      run: |
        cmake -B build -DCMAKE_BUILD_TYPE=${{ matrix.build_type }} 
              -DDIGIVICE_BUILD_TESTS=ON
              
    - name: Build
      run: cmake --build build --config ${{ matrix.build_type }}
      
    - name: Run Tests
      run: |
        cd build
        ctest --output-on-failure
        
    - name: Run Asset Validation
      run: |
        ./build/tools/validate_assets assets/
        
    - name: Performance Benchmarks
      if: matrix.build_type == 'Release'
      run: |
        ./build/tests/performance_tests --benchmark
```

## 2.4 Code Quality & Architecture

### Static Analysis Integration
```cpp
// .clang-tidy
Checks: >
  clang-analyzer-*,
  bugprone-*,
  performance-*,
  readability-*,
  modernize-*,
  -readability-magic-numbers

CheckOptions:
  - key: readability-identifier-naming.ClassCase
    value: CamelCase
  - key: readability-identifier-naming.MemberCase  
    value: camelBack
  - key: readability-identifier-naming.PrivateMemberSuffix
    value: '_'
```

### Architecture Documentation
```cpp
// docs/ARCHITECTURE_DECISION_RECORDS.md
# Architecture Decision Records (ADRs)

## ADR-001: State Management Pattern
**Status**: Accepted
**Date**: 2024-XX-XX

### Context
Need clean separation between game states while maintaining shared resources.

### Decision
Use stack-based state management with GameState base class.

### Consequences
- Positive: Clean state transitions, easy debugging
- Negative: More complex than single state machine
- Mitigation: Comprehensive state transition testing

## ADR-002: Asset Management Strategy  
**Status**: Accepted
**Date**: 2024-XX-XX

### Context
Balance memory usage with loading performance.

### Decision
Centralized AssetManager with reference counting and LRU eviction.

### Consequences
- Positive: Controlled memory usage, shared resource optimization
- Negative: More complex than simple preloading
- Mitigation: Asset validation tools and memory profiling
```

### Code Standards Documentation
```cpp
// docs/CODING_STANDARDS.md
# DigiviceRefactor Coding Standards

## Naming Conventions
- Classes: `PascalCase` (e.g., `AssetManager`)
- Methods: `camelCase` (e.g., `loadTexture()`)
- Private members: `camelCase_` (e.g., `textureCache_`)
- Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_TEXTURES`)

## Error Handling
- Use ErrorManager for all error reporting
- Provide graceful degradation when possible
- Document error conditions in function headers

## Performance Guidelines
- Profile before optimizing
- Prefer cache-friendly data structures
- Minimize dynamic allocations in hot paths
- Use const-correctness throughout
```

## Implementation Timeline

### Week 1: Testing Foundation
- [ ] Setup Catch2 testing framework
- [ ] Create GameTestFramework base classes
- [ ] Write initial unit tests for core systems
- [ ] Setup CI pipeline with basic build testing

### Week 2: Development Tools
- [ ] Implement DebugOverlay system
- [ ] Create asset validation tools
- [ ] Add performance profiling infrastructure
- [ ] Enhanced CMake configuration

### Week 3: Quality & Documentation
- [ ] Static analysis integration
- [ ] Architecture decision records
- [ ] Comprehensive code documentation
- [ ] Performance benchmarking suite

### Week 4: Integration & Polish
- [ ] Full CI/CD pipeline
- [ ] Automated asset validation
- [ ] Performance regression testing
- [ ] Developer workflow documentation

## Success Metrics

1. **Test Coverage**: >80% code coverage for core systems
2. **Build Time**: <5 minutes for full rebuild  
3. **CI Pipeline**: <15 minutes end-to-end
4. **Code Quality**: Zero critical static analysis issues
5. **Documentation**: Complete API documentation

## Risk Mitigation

- Start with high-value, low-risk tests (state transitions)
- Implement tools incrementally to avoid disrupting current development
- Maintain compatibility with existing development workflow
- Create rollback plan if tools introduce instability
