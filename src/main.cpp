#include <Arduino.h>
#include <FastLED.h>
#include "Starfield.h"

#define LED_PIN 2
#define WIDTH 20
#define HEIGHT 26
#define NUM_LEDS (WIDTH * HEIGHT)
#define LED_TYPE WS2811
#define COLOR_ORDER RGB

CRGB leds[NUM_LEDS];
const uint8_t globalBrightness = 255;
const uint16_t frameDelayMs = 5;

Starfield starfield(leds, WIDTH, HEIGHT);


void setup() {
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.clear(true);
    FastLED.setBrightness(globalBrightness);

    // Better random seed
    unsigned long seed = 0;
    for (uint8_t i = 0; i < 32; i++) {
        seed ^= analogRead(A0) << i;
        delay(1);
    }

    randomSeed(seed);
}

void loop() {
    static unsigned long lastMs = millis();
    unsigned long now = millis();
    unsigned long dtMs = now - lastMs;

    starfield.addStar();


    lastMs = now;

    starfield.update((float)dtMs / 1000.0f);
    starfield.render();

    FastLED.show();
    FastLED.delay(frameDelayMs);
}
