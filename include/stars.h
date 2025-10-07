#ifndef STARS_H
#define STARS_H


#include <Arduino.h>
#include "config.h"


struct Star {
    float x; // global continuous column position
    int row; // row index 0..CURTAIN_HEIGHT-1
    float vx; // columns per second
    float bright; // 0..1
};


extern Star *starsArr; // allocated to MAX_STARS


void starsInit();
void starsFree();
void resetStar(Star &s, bool randomRowAllowed=true);
void updateAndRenderStars(float dt);

bool addStar();



#endif // STARS_H