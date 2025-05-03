// File: include/platform/idisplay.h (Assuming this path is correct)
#pragma once

#include <cstdint> // Standard library - OK

class IDisplay {
public:
    virtual ~IDisplay() = default;

    virtual bool init(const char* title, int width, int height) = 0;
    virtual void clear(uint16_t color) = 0;
    // Added to get window dimensions for dynamic layout calculations
    void getWindowSize(int& width, int& height) const; // Changed to const as it doesn't modify the display object
    // Interface for raw pixel drawing (even if PC impl won't use it much)
    virtual void drawPixels(int dstX, int dstY,
                              int width, int height,
                              const uint16_t* srcData, int srcDataW, int srcDataH,
                              int srcX, int srcY) = 0;

    virtual void present() = 0;
    virtual void close() = 0;
};