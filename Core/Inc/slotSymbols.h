#ifndef TAO888_SLOTSYMBOLS
#define TAO888_SLOTSYMBOLS

#include "images.h"
#include <stdint.h>

typedef struct {
  Image_t image;
  uint8_t weight;
  uint8_t prizeMultiplier;
} SlotSymbol_t;

const SlotSymbol_t APPLE_SYMBOL = {
  APPLE_IMAGE,
  10,
  1
};
const SlotSymbol_t BANANA_SYMBOL = {
  BANANA_IMAGE,
  10,
  1
};
const SlotSymbol_t BLUEBERRY_SYMBOL = {
  BLUEBERRY_IMAGE,
  10,
  1
};
const SlotSymbol_t CHERRY_SYMBOL = {
  CHERRY_IMAGE,
  10,
  1
};
const SlotSymbol_t GRAPE_SYMBOL = {
  GRAPE_IMAGE,
  10,
  1
};
const SlotSymbol_t HAZELNUT_SYMBOL = {
  HAZELNUT_IMAGE,
  10,
  1
};
const SlotSymbol_t LIME_SYMBOL = {
  LIME_IMAGE,
  10,
  1
};
const SlotSymbol_t MANGOSTEEN_SYMBOL = {
  MANGOSTEEN_IMAGE,
  10,
  1
};
const SlotSymbol_t ORANGE_SYMBOl = {
  ORANGE_IMAGE,
  10,
  1
};
const SlotSymbol_t PEACH_SYMBOL = {
  PEACH_IMAGE,
  10,
  1
};
const SlotSymbol_t SCATTER_SYMBOL = {
  SCATTER_IMAGE,
  10,
  1
};
const SlotSymbol_t STRAWBERRY_SYMBOL = {
  STRAWBERRY_IMAGE,
  10,
  1
};
const SlotSymbol_t WATERMELON_SYMBOL = {
  WATERMELON_IMAGE,
  10,
  1
};
const SlotSymbol_t WILD_SYMBOL = {
  WILD_IMAGE,
  10,
  1
};

SlotSymbol_t slotSymbols[] = { APPLE_SYMBOL, BANANA_SYMBOL, BLUEBERRY_SYMBOL, CHERRY_SYMBOL, GRAPE_SYMBOL, HAZELNUT_SYMBOL, LIME_SYMBOL, MANGOSTEEN_SYMBOL, ORANGE_SYMBOl, PEACH_SYMBOL, SCATTER_SYMBOL, STRAWBERRY_SYMBOL, WATERMELON_SYMBOL, WILD_SYMBOL };

#endif