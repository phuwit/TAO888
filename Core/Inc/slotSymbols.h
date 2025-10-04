#ifndef TAO888_SLOTSYMBOLS
#define TAO888_SLOTSYMBOLS

#include "images.h"
#include <stdint.h>

typedef struct {
  Image_t image;
  uint8_t weight;
  uint8_t prizeMultiplier;
} SlotSymbol_t;

const SlotSymbol_t apple = {
  APPLE,
  10,
  1
};
const SlotSymbol_t banana = {
  BANANA,
  10,
  1
};
const SlotSymbol_t bluberry = {
  BLUEBERRY,
  10,
  1
};
const SlotSymbol_t cherry = {
  CHERRY,
  10,
  1
};
const SlotSymbol_t grape = {
  GRAPE,
  10,
  1
};
const SlotSymbol_t hazenut = {
  HAZENUT,
  10,
  1
};
const SlotSymbol_t lime = {
  LIME,
  10,
  1
};
const SlotSymbol_t mangosteen = {
  MANGOSTEEN,
  10,
  1
};
const SlotSymbol_t orange = {
  ORANGE,
  10,
  1
};
const SlotSymbol_t peach = {
  PEACH,
  10,
  1
};
const SlotSymbol_t scatter = {
  SCATTER,
  10,
  1
};
const SlotSymbol_t strawberry = {
  STRAWBERRY,
  10,
  1
};
const SlotSymbol_t watermelon = {
  WATERMELON,
  10,
  1
};
const SlotSymbol_t wild = {
  WILD,
  10,
  1
};

SlotSymbol_t slotSymbols[] = { apple, banana, bluberry, cherry, grape, hazenut, lime, mangosteen, orange, peach, scatter, strawberry, watermelon, wild };

#endif