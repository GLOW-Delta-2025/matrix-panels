#include "../include/renderer.h"
#include "../include/octo_wrapper.h"

static uint8_t *pixBuf = nullptr;


void rendererInit() {
    if (pixBuf) return;
    pixBuf = (uint8_t*) malloc((size_t)NUM_PIXELS * 3);
    if (!pixBuf) {
        Serial.println("ERROR: not enough RAM for pixBuf");
        while (1) delay(1000);
    }
    memset(pixBuf, 0, (size_t)NUM_PIXELS * 3);
}


void rendererFree() {
    if (pixBuf) free(pixBuf);
    pixBuf = nullptr;
}


void addPixelRGB_soft(int globalPixelIdx, float r, float g, float b) {
    if (!pixBuf) return;
    if (globalPixelIdx < 0 || globalPixelIdx >= NUM_PIXELS) return;
    int base = globalPixelIdx * 3;
    int v;

    v = (int)pixBuf[base + 0] + (int)r;
    if (v > 255) v = 255; pixBuf[base + 0] = (uint8_t)v;

    v = (int)pixBuf[base + 1] + (int)g;
    if (v > 255) v = 255; pixBuf[base + 1] = (uint8_t)v;

    v = (int)pixBuf[base + 2] + (int)b;
    if (v > 255) v = 255; pixBuf[base + 2] = (uint8_t)v;
}


void fadeBuffer() {
    if (!pixBuf) return;
    int total = NUM_PIXELS * 3;
    for (int i = 0; i < total; i++) {
        float v = (float)pixBuf[i] * fadeFactor;
        if (v < 0.5f) v = 0.0f;
        pixBuf[i] = (uint8_t)min(255.0f, v);
    }
}


void copyBufferToOcto() {
    if (!pixBuf) return;
    for (int curtain = 0; curtain < CURTAINS; curtain++) {
        for (int localIndex = 0; localIndex < LEDS_PER_CURTAIN; localIndex++) {
            int globalIdx = curtain * LEDS_PER_CURTAIN + localIndex;
            int base = globalIdx * 3;
            uint8_t r = pixBuf[base + 0];
            uint8_t g = pixBuf[base + 1];
            uint8_t b = pixBuf[base + 2];
            octoSetPixel(globalIdx, r, g, b);
        }
    }
}