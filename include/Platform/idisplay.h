// File: include/Platform/idisplay.h

#pragma once

#include <cstdint> // Standard library - OK

class IDisplay {
public:
    virtual ~IDisplay() = default;

    virtual bool init(const char* title, int width, int height) = 0;
    virtual void clear(uint16_t color) = 0;

    // --- MODIFIED drawPixels ---
    // Draws a sub-rectangle from source data onto the screen.
    // dstX, dstY: Top-left corner on the screen destination.
    // width, height: Size of the rectangle to draw (on screen AND from source).
    // srcData: Pointer to the start of the *entire* source image data.
    // srcDataW, srcDataH: Full width and height of the *entire* source image data.
    // srcX, srcY: Top-left corner *within the source data* to start copying from.
    virtual void drawPixels(int dstX, int dstY,
                              int width, int height,
                              const uint16_t* srcData, int srcDataW, int srcDataH,
                              int srcX, int srcY) = 0;
    // --- End of modification ---

    virtual void present() = 0;
    virtual void close() = 0;
};