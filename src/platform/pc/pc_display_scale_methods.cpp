void PCDisplay::beginFrame() {
    if (!initialized_ || !renderer_ || !renderTarget_) return;
    
    // Set render target to our texture
    SDL_SetRenderTarget(renderer_, renderTarget_);
    
    // Clear the render target
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
}

void PCDisplay::endFrame() {
    if (!initialized_ || !renderer_ || !renderTarget_) return;
    
    // Reset to default render target (the window)
    SDL_SetRenderTarget(renderer_, nullptr);
    
    // Clear the main renderer
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
    
    // Get the scale factor from the graphics constants
    float scale = Digivice::GraphicsConstants::getAssetScale();
    
    // Calculate destination rect for scaled rendering
    int scaledWidth = static_cast<int>(nativeWidth_ * scale);
    int scaledHeight = static_cast<int>(nativeHeight_ * scale);
    
    // Center the scaled output in the window
    SDL_Rect destRect = {
        (width_ - scaledWidth) / 2,
        (height_ - scaledHeight) / 2,
        scaledWidth,
        scaledHeight
    };
    
    // Render the scaled game texture to the window
    SDL_RenderCopy(renderer_, renderTarget_, nullptr, &destRect);
    
    // Present the renderer (this will be called by the game loop)
}
