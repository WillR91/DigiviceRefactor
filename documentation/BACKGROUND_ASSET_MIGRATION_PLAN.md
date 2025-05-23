# Background Asset Migration Plan

## Overview

Migration from large upscaled background assets to 1x scale assets with dynamic scaling for memory optimization and variant support.

## Current vs New Asset Systems

### Current System
- **Location**: `assets/backgrounds/environments/`
- **Naming**: `layer_0.png`, `layer_1.png`, `layer_2.png`
- **Layer Mapping**:
  - `layer_0.png` = Foreground
  - `layer_1.png` = Middleground
  - `layer_2.png` = Background
- **Dimensions**: 1421×474 pixels (upscaled)
- **Memory Usage**: ~2.7MB per layer, ~8.1MB per environment
- **Format**: Individual PNG files

### New System
- **Location**: `assets/backgrounds/environmentsnew/`
- **Naming**: `{environment}_{layer}_v{variant}.png`
- **Layer Mapping**:
  - `*_fg_v1.png` = Foreground
  - `*_mg_v1.png` = Middleground
  - `*_bg_v1.png` = Background
  - `*_bg_v2.png`, `*_bg_v3.png` = Background variants
- **Dimensions**: 384×128 pixels (1x scale)
- **Memory Usage**: ~0.17MB per layer, ~0.5MB per environment
- **Memory Savings**: ~94% reduction
- **Format**: Sprite sheets with JSON metadata (TexturePacker format)

## Migration Phases

### Phase 1: Asset Analysis & Inventory

#### 1.1 Current Environment Mapping
Analyze `src/states/MapSystemState.cpp` to identify all currently loaded environments:

```cpp
// Current loading pattern to map:
node.adventureBackgroundLayers.push_back(BackgroundLayerData({
    "assets/backgrounds/environments/01_file_island/01_native_forest/layer_0.png"
}, 0.5f, 0.0f));
```

#### 1.2 New Asset Inventory
- Map current environments to new asset equivalents
- Count available variants per environment
- Identify any missing assets that need creation

#### 1.3 Scaling Requirements
- **Source**: 384×128 (1x scale)
- **Target**: Fill 466×466 square display
- **Required Scale Factor**: ~3.6x to fill height, ~1.2x to fill width
- **Strategy**: Use max scale to fill display completely (zoom to fill)

### Phase 2: Code Architecture Updates

#### 2.1 Update BackgroundLayerData Structure

**Current Structure:**
```cpp
struct BackgroundLayerData {
    std::vector<std::string> texturePaths;
    float parallaxFactorX;
    float parallaxFactorY;
};
```

**New Structure:**
```cpp
struct BackgroundLayerData {
    std::vector<std::string> foregroundPaths;    // All fg variants
    std::vector<std::string> middlegroundPaths;  // All mg variants  
    std::vector<std::string> backgroundPaths;    // All bg variants (v1, v2, v3...)
    float parallaxFactorX;
    float parallaxFactorY;
    
    // Variant selection
    int selectedForegroundVariant = 0;
    int selectedMiddlegroundVariant = 0;
    int selectedBackgroundVariant = 0;
};
```

#### 2.2 Add Variant Selection System

```cpp
// New class for managing background variants
class BackgroundVariantManager {
public:
    static int selectRandomVariant(const std::vector<std::string>& variants);
    static void initializeVariantsForNode(BackgroundLayerData& layerData);
    static std::string getSelectedPath(const std::vector<std::string>& variants, int selectedIndex);
    
private:
    static std::mt19937 rng_;
};
```

#### 2.3 Update AdventureState Loading

**Current Loading (in AdventureState constructor):**
```cpp
// Loads bgTexture0_, bgTexture1_, bgTexture2_ from single paths
```

**New Loading:**
```cpp
// Load selected variants for each layer
void AdventureState::loadBackgroundVariants() {
    if (!currentNode_) return;
    
    for (auto& layerData : currentNode_->adventureBackgroundLayers) {
        // Initialize random variants
        BackgroundVariantManager::initializeVariantsForNode(layerData);
        
        // Load selected textures
        if (!layerData.foregroundPaths.empty()) {
            std::string fgPath = BackgroundVariantManager::getSelectedPath(
                layerData.foregroundPaths, layerData.selectedForegroundVariant);
            bgTexture0_ = AssetManager::getInstance().getTexture(fgPath);
        }
        
        if (!layerData.middlegroundPaths.empty()) {
            std::string mgPath = BackgroundVariantManager::getSelectedPath(
                layerData.middlegroundPaths, layerData.selectedMiddlegroundVariant);
            bgTexture1_ = AssetManager::getInstance().getTexture(mgPath);
        }
        
        if (!layerData.backgroundPaths.empty()) {
            std::string bgPath = BackgroundVariantManager::getSelectedPath(
                layerData.backgroundPaths, layerData.selectedBackgroundVariant);
            bgTexture2_ = AssetManager::getInstance().getTexture(bgPath);
        }
    }
}
```

#### 2.4 Enhanced Scaling Renderer

```cpp
void AdventureState::render_scaled_background_layer(PCDisplay& display, SDL_Texture* texture, 
                                                   int screenWidth, int screenHeight, 
                                                   float globalScale, int layerIndex) {
    if (!texture) return;
    
    // Get 1x texture dimensions
    int originalWidth, originalHeight;
    SDL_QueryTexture(texture, nullptr, nullptr, &originalWidth, &originalHeight);
    
    // Calculate scaling to fill square display (384×128 → 466×466)
    float scaleX = static_cast<float>(screenWidth) / originalWidth;   // ~1.21x
    float scaleY = static_cast<float>(screenHeight) / originalHeight; // ~3.64x
    
    // Use larger scale to fill display completely (zoom to fill)
    float baseScale = std::max(scaleX, scaleY); // Will use ~3.64x
    
    // Apply user's global scale preference
    float finalScale = baseScale * globalScale;
    
    // Calculate final dimensions
    int scaledWidth = static_cast<int>(originalWidth * finalScale);   // ~1398px
    int scaledHeight = static_cast<int>(originalHeight * finalScale); // ~466px
    
    // Center the scaled background (will crop sides)
    int posX = (screenWidth - scaledWidth) / 2;  // Negative value crops sides
    int posY = (screenHeight - scaledHeight) / 2; // Should be 0
    
    // Apply parallax scrolling if available
    if (currentNode_ && layerIndex < currentNode_->adventureBackgroundLayers.size()) {
        const auto& layerData = currentNode_->adventureBackgroundLayers[layerIndex];
        posX += static_cast<int>(layerData.parallaxFactorX * bg_scroll_offset_1_);
        posY += static_cast<int>(layerData.parallaxFactorY * bg_scroll_offset_1_);
    }
    
    SDL_Rect destRect = { posX, posY, scaledWidth, scaledHeight };
    display.drawTexture(texture, nullptr, &destRect);
}
```

### Phase 3: Asset Path Migration

#### 3.1 Update MapSystemState Node Definitions

**Current Pattern:**
```cpp
node.adventureBackgroundLayers.push_back(BackgroundLayerData({
    "assets/backgrounds/environments/01_file_island/01_native_forest/layer_0.png"
}, 0.5f, 0.0f));
```

**New Pattern:**
```cpp
node.adventureBackgroundLayers.push_back(BackgroundLayerData{
    // Foreground variants
    {"assets/backgrounds/environmentsnew/01_file_island/01_tropicaljungle/tropicaljungle_fg_v1.png"},
    
    // Middleground variants  
    {"assets/backgrounds/environmentsnew/01_file_island/01_tropicaljungle/tropicaljungle_mg_v1.png"},
    
    // Background variants (multiple versions)
    {"assets/backgrounds/environmentsnew/01_file_island/01_tropicaljungle/tropicaljungle_bg_v1.png",
     "assets/backgrounds/environmentsnew/01_file_island/01_tropicaljungle/tropicaljungle_bg_v2.png"},
     
    0.5f,  // parallaxFactorX
    0.0f   // parallaxFactorY
});
```

#### 3.2 Environment Mapping Table

| Current Environment | New Environment Asset Prefix | Available Variants |
|-------------------|------------------------------|-------------------|
| 01_native_forest | tropicaljungle | bg: v1,v2 mg: v1 fg: v1 |
| 02_file_city | [TBD] | [TBD] |
| ... | ... | ... |

### Phase 4: Sprite Sheet Support (Future Enhancement)

#### 4.1 TexturePacker JSON Integration
The new assets come with JSON metadata. Future enhancement could include:

```cpp
// Support for sprite sheet loading
class SpriteSheetLoader {
public:
    struct FrameData {
        SDL_Rect sourceRect;
        SDL_Point pivot;
        bool rotated;
    };
    
    static std::map<std::string, FrameData> loadTexturePackerJSON(const std::string& jsonPath);
    static SDL_Texture* loadSpriteSheet(const std::string& pngPath);
};
```

### Phase 5: Implementation Strategy

#### 5.1 Gradual Migration Approach
1. **Phase 5.1**: Implement new loading system alongside current system
2. **Phase 5.2**: Add feature flag to switch between old/new assets
3. **Phase 5.3**: Migrate one environment at a time for testing
4. **Phase 5.4**: Complete migration and remove old asset support
5. **Phase 5.5**: Add variant randomization features

#### 5.2 Testing Checklist
- [ ] Memory usage verification (should see ~94% reduction)
- [ ] Visual quality comparison (1x scaled vs pre-upscaled)
- [ ] Performance impact of dynamic scaling
- [ ] Variant selection functionality
- [ ] Parallax scrolling still works correctly
- [ ] All environments load properly

### Phase 6: Memory Optimization Validation

#### 6.1 Expected Memory Savings
- **Before**: 1421×474×4 bytes × 3 layers = ~8.1MB per environment
- **After**: 384×128×4 bytes × 3 layers = ~0.5MB per environment  
- **Total Savings**: ~94% memory reduction
- **For 30 environments**: 243MB → 15MB (~228MB saved)

#### 6.2 Performance Considerations
- **GPU Scaling**: Dynamic texture scaling is very fast on GPU
- **CPU Load**: Minimal increase for scaling calculations
- **Loading Time**: Faster due to smaller file sizes
- **Variant Selection**: Negligible performance impact

## Risk Assessment

### Low Risk
- Memory optimization (proven approach)
- Dynamic scaling (standard GPU operation)
- Asset path changes (straightforward refactoring)

### Medium Risk  
- Visual quality differences (need comparison testing)
- Variant system complexity (new feature)
- Parallax scrolling interaction (needs validation)

### High Risk
- None identified (migration is well-defined)

## Success Criteria

1. **Memory Usage**: Achieve >90% reduction in background asset memory usage
2. **Visual Quality**: Maintain equivalent visual appearance to current system
3. **Functionality**: All current environments work with new asset system
4. **Performance**: No significant FPS impact from dynamic scaling
5. **Variants**: Successfully implement random background variant selection
6. **Maintainability**: Clean, well-documented code for future enhancements

## Future Enhancements

1. **Dynamic Variant Switching**: Change variants during gameplay
2. **Weather Effects**: Layer additional effects on backgrounds
3. **Time-based Variants**: Different variants for day/night cycles
4. **Animation Support**: Animated background elements
5. **Compression**: Further optimize with compressed texture formats

## Dependencies

- **Asset Creation**: All 1x scale assets available in environmentsnew/
- **Code Changes**: Updates to BackgroundLayerData, AdventureState, MapSystemState
- **Testing**: Visual comparison and memory profiling tools
- **Documentation**: Updated asset guidelines and variant specifications

---

**Document Version**: 1.0  
**Created**: [Current Date]  
**Last Updated**: [Current Date]  
**Status**: Planning Phase