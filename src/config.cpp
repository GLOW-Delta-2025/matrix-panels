#include "../include/config.h"

// per-curtain inversion (change as-needed)
bool invertCurtain[CURTAINS] = { false, false, false, false, false };

// runtime tunables default values
const int MAX_STARS = 500; // maximum alloc size - change higher if you have RAM
int activeStarCount = 0; // initial active stars (<= MAX_STARS)


float minSpeedColsPerSec = 8.0f;
float maxSpeedColsPerSec = 25.0f;
float fadeFactor = 0.86f;
unsigned long frameTargetMs = 1; // ~50 FPS
bool randomRows = true;
bool wrapStars = false;

uint8_t STAR_R = 255;
uint8_t STAR_G = 191;
uint8_t STAR_B = 3;

// pin list - only used when USE_PINLIST is true
const byte pinList[CURTAINS] = {  7, 6, 14, 2, 8 };
