#include "../include/config.h"

// per-curtain inversion (change as-needed)
bool invertCurtain[CURTAINS] = { false, false, false, false, false };

// runtime tunables default values
const int MAX_STARS = 500; // maximum alloc size - change higher if you have RAM
int activeStarCount = 0; // initial active stars (<= MAX_STARS)


// Baseline speeds - these are the true defaults that effects should restore to
const float BASELINE_MIN_SPEED = 8.0f;
const float BASELINE_MAX_SPEED = 25.0f;

float minSpeedColsPerSec = BASELINE_MIN_SPEED;
float maxSpeedColsPerSec = BASELINE_MAX_SPEED;
float fadeFactor = 0.86f;
unsigned long frameTargetMs = 1; // ~50 FPS
bool randomRows = true;
bool wrapStars = false;

uint8_t STAR_R = 255;
uint8_t STAR_G = 191;
uint8_t STAR_B = 3;

// pin list - only used when USE_PINLIST is true
const byte pinList[CURTAINS] = {  7, 6, 14, 2, 8 };
