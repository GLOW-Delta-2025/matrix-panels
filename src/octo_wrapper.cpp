#include "../include/octo_wrapper.h"


DMAMEM int displayMemory[LEDS_PER_CURTAIN * 6];
int drawingMemory[LEDS_PER_CURTAIN * 6];
const int config_flags = WS2811_GRB | WS2811_800kHz;

#if USE_PINLIST
OctoWS2811 leds(LEDS_PER_CURTAIN, displayMemory, drawingMemory, config_flags, CURTAINS, (byte*)pinList);
#else
OctoWS2811 leds(LEDS_PER_CURTAIN, displayMemory, drawingMemory, config_flags);
#endif


void octoBegin() {
    leds.begin();
    leds.show();
}


void octoShow() {
    leds.show();
}


void octoSetPixel(int globalIdx, uint8_t r, uint8_t g, uint8_t b) {
    leds.setPixel(globalIdx, r, g, b);
}