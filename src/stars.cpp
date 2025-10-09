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
    resetStar(starsArr[i], true);
    // spread initial x so they don't all appear at once
    starsArr[i].x = random(0, TOTAL_WIDTH * 100) / 100.0f;
  }
}

void starsFree() {
  if (starsArr) free(starsArr);
  starsArr = nullptr;
  starsAllocated = false;
}

void resetStar(Star &s, bool randomRowAllowed) {
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

  uint8_t r, g, b;

  if (s.r != 0 || s.g != 0 || s.b != 0 && (s.r != STAR_R || s.g != STAR_G || s.b != STAR_B)) {
    Serial.println("Star has custom color");
    Serial.println(s.r);
    Serial.println(s.g);
    Serial.println(s.b);
    r = s.r;
    g = s.g;
    b = s.b;
  } else {
    r = STAR_R;
    g = STAR_G;
    b = STAR_B;
  }

  float rL = r * br * wl;
  float gL = g * br * wl;
  float bL = b * br * wl;
  float rR = r * br * wr;
  float gR = g * br * wr;
  float bR = b * br * wr;

  if (leftCol >= 0 && leftCol < TOTAL_WIDTH) {
    int curtainL = leftCol / CURTAIN_WIDTH;
    int localColL = leftCol % CURTAIN_WIDTH;
    int row = s.row;
    if (curtainL >= 0 && curtainL < CURTAINS) {
      int rowOut = invertCurtain[curtainL] ? (CURTAIN_HEIGHT - 1 - row) : row;
      int localIndex = localIndexInCurtain(localColL, rowOut);
      int globalIdx = globalOctoIndex(curtainL, localIndex);
      addPixelRGB_soft(globalIdx, rL, gL, bL);
    }
  }

  int rightCol = leftCol + 1;
  if (rightCol >= 0 && rightCol < TOTAL_WIDTH) {
    int curtainR = rightCol / CURTAIN_WIDTH;
    int localColR = rightCol % CURTAIN_WIDTH;
    int row = s.row;
    if (curtainR >= 0 && curtainR < CURTAINS) {
      int rowOut = invertCurtain[curtainR] ? (CURTAIN_HEIGHT - 1 - row) : row;
      int localIndex = localIndexInCurtain(localColR, rowOut);
      int globalIdx = globalOctoIndex(curtainR, localIndex);
      addPixelRGB_soft(globalIdx, rR, gR, bR);
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
        resetStar(s, true);
      }
    }
  }
}

bool addStar() {
  Star &s = starsArr[activeStarCount];
  resetStar(s, true);
  // start slightly left so the star slides in smoothly
  s.x = - (random(0, 50) / 25.0f);
  activeStarCount++;
  return true;
}
