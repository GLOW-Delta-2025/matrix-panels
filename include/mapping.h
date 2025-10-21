#ifndef MAPPING_H
#define MAPPING_H

#include "config.h"

inline int localIndexInCurtain(int col, int row) {
    return col * CURTAIN_HEIGHT + row;
}

inline int globalOctoIndex(int curtainIdx, int localIndex) {
    return curtainIdx * LEDS_PER_CURTAIN + localIndex;
}

#endif // MAPPING_H