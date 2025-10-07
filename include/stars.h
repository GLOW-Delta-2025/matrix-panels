#ifndef STARS_H
#define STARS_H


#include <Arduino.h>
#include "config.h"


struct Star {
    float x; // global continuous column position
    int row; // row index 0..CURTAIN_HEIGHT-1
    float vx; // columns per second
    float bright; // 0..1
    uint8_t r = 0, g = 0, b = 0;
};


extern Star *starsArr; // allocated to MAX_STARS


void starsInit();
void starsFree();
void resetStar(Star &s, bool randomRowAllowed=true);
void updateAndRenderStars(float dt);

bool addStar();
bool addStar(int row, float vx, float bright, uint8_t r, uint8_t g, uint8_t b);


#endif // STARS_H