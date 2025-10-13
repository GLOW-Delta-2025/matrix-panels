#include "stars.h"
#include "renderer.h"
#include "mapping.h"
#include "../include/config.h"

static bool starsAllocated = false;
Star *starsArr = nullptr;
unsigned long lastMicros_local = 0;

void starsInit() {
  if (starsAllocated) return;
  starsArr = (Star*) malloc(sizeof(Star) * MAX_STARS);
  if (!starsArr) {
    Serial.println("ERROR: not enough RAM for stars array");
    while (1) delay(1000);
  }
  starsAllocated = true;

  randomSeed(analogRead(A0) ^ micros());
  // initialize
  for (int i = 0; i < MAX_STARS; i++) {
    randomizeStarProperties(starsArr[i], true);
    // spread initial x so they don't all appear at once
    starsArr[i].x = random(0, TOTAL_WIDTH * 100) / 100.0f;
  }
}

void starsFree() {
  if (starsArr) free(starsArr);
  starsArr = nullptr;
  starsAllocated = false;
}

void randomizeStarProperties(Star &s, bool randomRowAllowed) {
  s.x = - (random(0, 50) / 25.0f); // -0 .. -2
  if (randomRows && randomRowAllowed) s.row = random(0, CURTAIN_HEIGHT);
  else s.row = 0;
  s.vx = random((int)(minSpeedColsPerSec * 100.0f), (int)(maxSpeedColsPerSec * 100.0f)) / 100.0f;
  s.bright = random(70, 101) / 100.0f; // 0.70 .. 1.00
}



// render a single star into the soft buffer
static void renderStarToBuffer(const Star &s) {
  float fx = s.x;
  int leftCol = (int)floorf(fx);
  float frac = fx - leftCol;
  float wl = 1.0f - frac;
  float wr = frac;
  float br = s.bright;

  // Process the star and its trail based on size
  for (int i = 0; i < s.size; i++) {
    // Calculate brightness falloff for trail
    float trailFactor = 1.0f - (float)i / s.size;
    
    // Position for this trail segment
    float trailX = fx - i * 0.5f;
    int trailLeftCol = (int)floorf(trailX);
    float trailFrac = trailX - trailLeftCol;
    float trailWl = 1.0f - trailFrac;
    float trailWr = trailFrac;
    
    // Adjusted brightness for trail segment
    float segmentBr = br * trailFactor;
    
    float rL = s.r * segmentBr * trailWl;
    float gL = s.g * segmentBr * trailWl;
    float bL = s.b * segmentBr * trailWl;
    float rR = s.r * segmentBr * trailWr;
    float gR = s.g * segmentBr * trailWr;
    float bR = s.b * segmentBr * trailWr;

    // Render left pixel of trail segment
    if (trailLeftCol >= 0 && trailLeftCol < TOTAL_WIDTH) {
      int curtainL = trailLeftCol / CURTAIN_WIDTH;
      int localColL = trailLeftCol % CURTAIN_WIDTH;
      int row = s.row;
      if (curtainL >= 0 && curtainL < CURTAINS) {
        int rowOut = invertCurtain[curtainL] ? (CURTAIN_HEIGHT - 1 - row) : row;
        int localIndex = localIndexInCurtain(localColL, rowOut);
        int globalIdx = globalOctoIndex(curtainL, localIndex);
        addPixelRGB_soft(globalIdx, rL, gL, bL);
      }
    }

    // Render right pixel of trail segment
    int trailRightCol = trailLeftCol + 1;
    if (trailRightCol >= 0 && trailRightCol < TOTAL_WIDTH) {
      int curtainR = trailRightCol / CURTAIN_WIDTH;
      int localColR = trailRightCol % CURTAIN_WIDTH;
      int row = s.row;
      if (curtainR >= 0 && curtainR < CURTAINS) {
        int rowOut = invertCurtain[curtainR] ? (CURTAIN_HEIGHT - 1 - row) : row;
        int localIndex = localIndexInCurtain(localColR, rowOut);
        int globalIdx = globalOctoIndex(curtainR, localIndex);
        addPixelRGB_soft(globalIdx, rR, gR, bR);
      }
    }
  }
}

void updateAndRenderStars(float dt) {
  if (!starsArr) return;
  for (int i = 0; i < activeStarCount; i++) {
    Star &s = starsArr[i];
    s.x += s.vx * dt;
    if (s.x > -2.0f && s.x < TOTAL_WIDTH + 2.0f) {
      renderStarToBuffer(s);
    }
    if (s.x > TOTAL_WIDTH + 1.0f) {
      if (wrapStars) {
        s.x -= (TOTAL_WIDTH + 2.0f);
      } else {
        randomizeStarProperties(s, true);
      }
    }
  }
}

bool addStar(float speed = -1, int hexColor = -1, int brightness = -1, int size = -1) {
  Star &s = starsArr[activeStarCount];
  randomizeStarProperties(s, true);

  if (speed != -1) {
    s.vx = speed;
  }

  if (hexColor != -1) {
    Serial.println("hexColor");
    Serial.println(hexColor);
    s.r = (hexColor >> 16) & 0xFF;  // Integer value 0-255
    s.g = (hexColor >> 8) & 0xFF;   // Integer value 0-255
    s.b = hexColor & 0xFF;
    Serial.println("Color");
    Serial.println(s.r);
    Serial.println(s.g);
    Serial.println(s.b);
  }

  if (brightness != -1) {
    s.bright = brightness / 100.0f;
  }

  if (size != -1) {
    s.size = size;
  }

  // start slightly left so the star slides in smoothly
  s.x = - (random(0, 50) / 25.0f);
  activeStarCount++;
  return true;
}