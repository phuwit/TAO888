#ifndef TAO888_SLOTSYMBOLS
#define TAO888_SLOTSYMBOLS

#include "images.h"
#include <stdint.h>

typedef struct {
  Image_t image;
  uint8_t weight;
  uint8_t prizeMultiplier;
} SlotSymbol_t;

const SlotSymbol_t cherry = {
  CHERRY_BLACKOUTLINE_ONWHITE,
  10,
  1
};

SlotSymbol_t slotSymbols[] = { cherry };

#endif