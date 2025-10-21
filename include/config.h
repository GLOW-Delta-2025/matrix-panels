#ifndef OCTO_CONFIG_H
#define OCTO_CONFIG_H
#include <Arduino.h>

#define CURTAINS 5
#define CURTAIN_WIDTH 20
#define CURTAIN_HEIGHT 26

#define TOTAL_WIDTH (CURTAINS * CURTAIN_WIDTH)
#define TOTAL_HEIGHT (CURTAIN_HEIGHT)
#define LEDS_PER_CURTAIN (CURTAIN_WIDTH * CURTAIN_HEIGHT)
#define NUM_PIXELS (CURTAINS * LEDS_PER_CURTAIN)

// Per-curtain row inversion (set in config.cpp)
extern bool invertCurtain[CURTAINS];

// runtime tunables (modifiable via serial reader)
extern int activeStarCount; // number of stars currently active (<= MAX_STARS)
extern const int MAX_STARS; // hard cap for allocation
extern float minSpeedColsPerSec; // min speed (cols/sec)
extern float maxSpeedColsPerSec; // max speed (cols/sec)
extern float fadeFactor; // per-frame fade (0..1)
extern unsigned long frameTargetMs;
extern bool randomRows;
extern bool wrapStars;

// default star color (modifiable)
extern uint8_t STAR_R, STAR_G, STAR_B;

// pin list (if USE_PINLIST)
extern const byte pinList[CURTAINS];

#endif // OCTO_CONFIG_H