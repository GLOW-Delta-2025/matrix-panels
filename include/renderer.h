#ifndef RENDERER_H
#define RENDERER_H


#include <Arduino.h>
#include "config.h"


void rendererInit();
void rendererFree();
void fadeBuffer();
void copyBufferToOcto();
void addPixelRGB_soft(int globalPixelIdx, float r, float g, float b);


#endif // RENDERER_H