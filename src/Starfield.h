//
// Created by omi on 10/1/25.
//
#pragma once
#include <FastLED.h>
#include "Star.h"
#include <vector>

#ifndef STARFIELD_H
#define STARFIELD_H

class Starfield {
public:
    Starfield(CRGB* leds, uint8_t width, uint8_t height);

    void spawnIfDue(unsigned long nowMs);
    void update(float dtSec);
    void render();
    void addStar();
    void removeStar();

    void setStarColor(const CRGB& color) { starColor = color; }

private:
    void resetStar(Star &s);

    CRGB* leds;  // Changed from uint32_t* to CRGB*
    uint8_t width;
    uint8_t height;

    std::vector<Star> stars;

    unsigned long nextSpawnAtMs = 0;

    // Config
    const uint16_t minSpawnIntervalMs = 180;
    const uint16_t maxSpawnIntervalMs = 450;
    const float minSpeedPxPerSec = 18.0f;
    const float maxSpeedPxPerSec = 30.0f;
    const uint8_t tailLength = 3;
    const uint8_t tailFalloff[3] = {255, 96, 32};
    const uint8_t frameFade = 40;

    CRGB starColor = CRGB::Orange;
};

#endif //STARFIELD_H