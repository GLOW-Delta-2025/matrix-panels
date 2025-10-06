#include <Arduino.h>
#include <FastLED.h>

// Pin definitions for 5 curtains
#define LED_PIN_1 7
#define LED_PIN_2 8
#define LED_PIN_3 9
#define LED_PIN_4 10
#define LED_PIN_5 11

// Dimensions per curtain
#define WIDTH 20
#define HEIGHT 26
#define NUM_LEDS_PER_CURTAIN (WIDTH * HEIGHT)
#define NUM_CURTAINS 5
#define TOTAL_WIDTH (WIDTH * NUM_CURTAINS)
#define TOTAL_LEDS (NUM_LEDS_PER_CURTAIN * NUM_CURTAINS)

#define LED_TYPE WS2811
#define COLOR_ORDER GRB

// LED arrays for each curtain
CRGB leds1[NUM_LEDS_PER_CURTAIN];
CRGB leds2[NUM_LEDS_PER_CURTAIN];
CRGB leds3[NUM_LEDS_PER_CURTAIN];
CRGB leds4[NUM_LEDS_PER_CURTAIN];
CRGB leds5[NUM_LEDS_PER_CURTAIN];

// Array of pointers for easy access
CRGB* ledCurtains[NUM_CURTAINS] = {leds1, leds2, leds3, leds4, leds5};

// Visual parameters
const uint8_t globalBrightness = 200;  // Reduced from 255 for faster updates
const CRGB starColor = CRGB::Orange;

// Timing - Optimized for 24 FPS
const uint16_t targetFPS = 24;
const uint16_t frameTimeUs = 1000000 / targetFPS;  // Microseconds per frame
unsigned long lastFrameUs = 0;

// Star system
struct Star {
    float x;      // sub-pixel horizontal position (0..TOTAL_WIDTH)
    uint8_t y;    // row 0..HEIGHT-1
    float vx;     // pixels per second
    bool active;
};

const uint8_t MAX_STARS = 8;  // Reduced slightly for performance
Star stars[MAX_STARS];

// Spawn control
const uint16_t minSpawnIntervalMs = 200;
const uint16_t maxSpawnIntervalMs = 500;
unsigned long nextSpawnAtMs = 0;

// Motion and tails - Optimized
const float minSpeedPxPerSec = 20.0f;
const float maxSpeedPxPerSec = 32.0f;
const uint8_t tailLength = 3;
const uint8_t tailFalloff[3] = {255, 100, 35};
const uint8_t frameFade = 45;  // Slightly increased for smoother fade

// FPS measurement
unsigned long frameCount = 0;
unsigned long fpsTimer = 0;
float currentFPS = 0.0;
unsigned long maxFrameTime = 0;
unsigned long totalFrameTime = 0;

// Dirty rectangle tracking for optimization
uint8_t minDirtyX = 255;
uint8_t maxDirtyX = 0;
bool hasDirtyPixels = false;

// Optimized pixel setting with dirty tracking
static inline void setPixelFast(int globalX, uint8_t y, CRGB color) {
    if (globalX < 0 || globalX >= TOTAL_WIDTH) return;

    uint8_t curtainIdx = globalX / WIDTH;
    uint8_t localX = globalX % WIDTH;

    // Track dirty regions
    if (curtainIdx < minDirtyX) minDirtyX = curtainIdx;
    if (curtainIdx > maxDirtyX) maxDirtyX = curtainIdx;
    hasDirtyPixels = true;

    // Direct array access (faster than function calls)
    uint16_t idx = localX * HEIGHT + y;
    ledCurtains[curtainIdx][idx] += color;
}

static inline uint16_t fastRand16(uint16_t max) {
    return (uint16_t)(((uint32_t)random() * max) >> 15);
}

static inline float fastRandFloat(float min, float max) {
    return min + (float)random() / (float)RAND_MAX * (max - min);
}

void resetStar(Star &s) {
    s.active = false;
}

void spawnStarIfDue(unsigned long nowMs) {
    if (nowMs < nextSpawnAtMs) return;

    // Find free slot
    for (uint8_t i = 0; i < MAX_STARS; i++) {
        if (!stars[i].active) {
            stars[i].active = true;
            stars[i].x = 0.0f;
            stars[i].y = fastRand16(HEIGHT);
            stars[i].vx = fastRandFloat(minSpeedPxPerSec, maxSpeedPxPerSec);
            break;
        }
    }

    nextSpawnAtMs = nowMs + minSpawnIntervalMs + fastRand16(maxSpawnIntervalMs - minSpawnIntervalMs);
}

void updateAndRenderStars(float dtSec) {
    // Reset dirty tracking
    minDirtyX = 255;
    maxDirtyX = 0;
    hasDirtyPixels = false;

    // Fade only dirty curtains from previous frame
    static uint8_t lastMinDirty = 0, lastMaxDirty = NUM_CURTAINS - 1;
    for (uint8_t c = lastMinDirty; c <= lastMaxDirty && c < NUM_CURTAINS; c++) {
        fadeToBlackBy(ledCurtains[c], NUM_LEDS_PER_CURTAIN, frameFade);
    }

    // Combined update and render pass
    for (uint8_t i = 0; i < MAX_STARS; i++) {
        if (!stars[i].active) continue;

        // Update position
        stars[i].x += stars[i].vx * dtSec;

        // Check bounds
        if (stars[i].x >= (float)(TOTAL_WIDTH + tailLength)) {
            resetStar(stars[i]);
            continue;
        }

        // Render immediately after update (better cache locality)
        int headX = (int)stars[i].x;
        uint8_t y = stars[i].y;

        // Unrolled tail loop for speed
        if (headX >= 0 && headX < TOTAL_WIDTH) {
            CRGB c = starColor;
            c.nscale8_video(tailFalloff[0]);
            setPixelFast(headX, y, c);
        }

        if (headX - 1 >= 0 && headX - 1 < TOTAL_WIDTH) {
            CRGB c = starColor;
            c.nscale8_video(tailFalloff[1]);
            setPixelFast(headX - 1, y, c);
        }

        if (headX - 2 >= 0 && headX - 2 < TOTAL_WIDTH) {
            CRGB c = starColor;
            c.nscale8_video(tailFalloff[2]);
            setPixelFast(headX - 2, y, c);
        }
    }

    // Store dirty region for next frame
    lastMinDirty = minDirtyX;
    lastMaxDirty = maxDirtyX;
}

void setup() {
    // Initialize Serial for monitoring
    Serial.begin(115200);
    Serial.println(F("LED Curtain - Optimized for 24 FPS"));
    Serial.print(F("Total LEDs: "));
    Serial.println(TOTAL_LEDS);

    // Configure FastLED for performance
    FastLED.setMaxRefreshRate(targetFPS);
    FastLED.setCorrection(TypicalLEDStrip);
    FastLED.setDither(0);  // Disable dithering for speed

    // Initialize all LED strips
    FastLED.addLeds<LED_TYPE, LED_PIN_1, COLOR_ORDER>(leds1, NUM_LEDS_PER_CURTAIN);
    FastLED.addLeds<LED_TYPE, LED_PIN_2, COLOR_ORDER>(leds2, NUM_LEDS_PER_CURTAIN);
    FastLED.addLeds<LED_TYPE, LED_PIN_3, COLOR_ORDER>(leds3, NUM_LEDS_PER_CURTAIN);
    FastLED.addLeds<LED_TYPE, LED_PIN_4, COLOR_ORDER>(leds4, NUM_LEDS_PER_CURTAIN);
    FastLED.addLeds<LED_TYPE, LED_PIN_5, COLOR_ORDER>(leds5, NUM_LEDS_PER_CURTAIN);

    FastLED.clear(true);
    FastLED.setBrightness(globalBrightness);

    // Initialize RNG with better seed
    randomSeed(analogRead(0) + analogRead(1) * 256);

    // Initialize stars
    for (uint8_t i = 0; i < MAX_STARS; i++) {
        resetStar(stars[i]);
    }

    nextSpawnAtMs = millis();
    fpsTimer = millis();
    lastFrameUs = micros();
}

void loop() {
    unsigned long nowUs = micros();
    unsigned long frameStartMs = millis();

    // Calculate precise delta time
    float dtSec = (float)(nowUs - lastFrameUs) / 1000000.0f;
    lastFrameUs = nowUs;

    // Update and render in one pass
    spawnStarIfDue(frameStartMs);
    updateAndRenderStars(dtSec);

    // Show the LEDs
    unsigned long showStart = micros();
    FastLED.show();
    unsigned long showTime = micros() - showStart;

    // FPS calculation and reporting
    frameCount++;
    if (frameStartMs - fpsTimer >= 1000) {
        currentFPS = frameCount * 1000.0 / (frameStartMs - fpsTimer);

        Serial.print(F("FPS: "));
        Serial.print(currentFPS, 1);
        Serial.print(F(" | Target: "));
        Serial.print(targetFPS);
        Serial.print(F(" | Show: "));
        Serial.print(showTime / 1000.0, 1);
        Serial.print(F("ms | Max frame: "));
        Serial.print(maxFrameTime / 1000.0, 1);
        Serial.print(F("ms | Avg: "));
        Serial.print((totalFrameTime / frameCount) / 1000.0, 1);
        Serial.println(F("ms"));

        frameCount = 0;
        fpsTimer = frameStartMs;
        maxFrameTime = 0;
        totalFrameTime = 0;
    }

    // Track frame time
    unsigned long frameTime = micros() - nowUs;
    totalFrameTime += frameTime;
    if (frameTime > maxFrameTime) maxFrameTime = frameTime;

    // Precise frame timing - only delay if we're ahead of schedule
    unsigned long targetNextFrame = lastFrameUs + frameTimeUs;
    if (micros() < targetNextFrame) {
        // Use delayMicroseconds for precision
        unsigned long delayNeeded = targetNextFrame - micros();
        if (delayNeeded > 16383) {
            delay(delayNeeded / 1000);
        } else if (delayNeeded > 0) {
            delayMicroseconds(delayNeeded);
        }
    }
}