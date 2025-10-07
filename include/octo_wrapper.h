#ifndef OCTO_WRAPPER_H
#define OCTO_WRAPPER_H


#include <OctoWS2811.h>
#include "config.h"


// OctoWS memory: sized by LEDS_PER_CURTAIN
extern DMAMEM int displayMemory[LEDS_PER_CURTAIN * 6];
extern int drawingMemory[LEDS_PER_CURTAIN * 6];


// leds object and helpers
extern OctoWS2811 leds;


void octoBegin();
void octoShow();
void octoSetPixel(int globalIdx, uint8_t r, uint8_t g, uint8_t b);


#endif // OCTO_WRAPPER_H