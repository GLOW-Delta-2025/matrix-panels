#include "Starfield.h"
#include <Arduino.h>
#include <math.h>

#include "Utils.h"

Starfield::Starfield(CRGB* leds, uint8_t width, uint8_t height)
    : leds(leds), width(width), height(height)
{
    // Start with no stars initially
    nextSpawnAtMs = millis();
}
void Starfield::resetStar(Star &s) {
    s.active = false;
    s.x = 0.0f;
    s.y = 0;
    s.vx = 0.0f;
}

void Starfield::spawnIfDue(unsigned long nowMs) {
    if (nowMs < nextSpawnAtMs) return;

    addStar();
    nextSpawnAtMs = nowMs + randBetween(minSpawnIntervalMs, maxSpawnIntervalMs);
}

void Starfield::update(float dtSec) {
    // Scale down the time delta to slow movement
    float scaledDt = dtSec * 0.3f;
    
    if (stars.size() > 0) {
        Serial.print("Update: dtSec=");
        Serial.print(scaledDt);
        Serial.print(", star count=");
        Serial.println(stars.size());
    }
    
    for (auto it = stars.begin(); it != stars.end();) {
        // Move star according to its velocity, with scaled time
        it->x += it->vx * scaledDt;
        
        // Occasionally drift up (less frequently)
        if (it->y > 0 && random(0, 100) < 5) {
            it->y -= 1;
        }
        
        // Remove stars that have moved off screen
        if (it->x >= (float)width) {
            Serial.println("Star removed (moved off screen)");
            it = stars.erase(it);
        } else {
            ++it;
        }
    }
}

void Starfield::render() {
    fadeToBlackBy(leds, width * height, frameFade);

    for (const Star& star : stars) {
        int headX = (int)floorf(star.x);
        uint8_t y = star.y;

        // Only display debug for stars near the origin
        if (headX < 2) {
            Serial.print("Rendering star: x=");
            Serial.print(star.x);
            Serial.print(", y=");
            Serial.println(y);
        }

        // Make sure headX is always within bounds
        if (headX >= width) {
            headX = width - 1;
        }

        for (uint8_t seg = 0; seg < tailLength; seg++) {
            int x = headX - (int)seg;
            if (x < 0 || x >= (int)width) continue;

            uint16_t idx = xyToIndex((uint8_t)x, y, height);
            CRGB c = starColor;
            c.nscale8_video(tailFalloff[seg]);
            leds[idx] += c;
        }
    }
}

void Starfield::addStar() {
    Star newStar;
    newStar.active = true;
    newStar.x = 0.0f;
    newStar.y = random(0, height);
    newStar.vx = random(minSpeedPxPerSec*100, maxSpeedPxPerSec*100) / 100.0f;
    
    // Make sure the velocity is positive so the star moves across the screen
    if (newStar.vx < 0.1f) newStar.vx = minSpeedPxPerSec;
    
    stars.push_back(newStar);
    
    // Debug output
    Serial.print("Added star: y=");
    Serial.print(newStar.y);
    Serial.print(", vx=");
    Serial.println(newStar.vx);
}

void Starfield::removeStar() {
    if (!stars.empty()) {
        stars.pop_back();
        Serial.println("Removed star");
    } else {
        Serial.println("No stars to remove");
    }
}