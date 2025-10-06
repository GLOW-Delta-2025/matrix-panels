//
// Created by omi on 10/1/25.
//

#ifndef UTILS_H
#define UTILS_H
#include <cstdint>
#include <cstdlib>

static inline uint8_t clampu8(int v) { return v < 0 ? 0 : (v > 255 ? 255 : (uint8_t)v); }

// Column-major mapping: index = x*HEIGHT + y
static inline uint16_t xyToIndex(uint8_t x, uint8_t y, uint8_t height) {
    return x * height + y;
}

static inline uint16_t randBetween(uint16_t a, uint16_t b) {
    return a + (rand() % (b - a + 1));
}

static inline float randFloat(float a, float b) {
    return a + (float)rand() / (float)RAND_MAX * (b - a);
}

#endif //UTILS_H
