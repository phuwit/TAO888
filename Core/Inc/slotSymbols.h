#ifndef TAO888_SLOTSYMBOLS
#define TAO888_SLOTSYMBOLS

#include "images.h"

typedef struct {
  Image image;
  uint8_t weight;
  uint8_t prizeMultiplier;
} SlotSymbol;

static const SlotSymbol APPLE_SYMBOL = {
  APPLE_IMAGE,
  10,
  1
};
static const SlotSymbol BANANA_SYMBOL = {
  BANANA_IMAGE,
  10,
  1
};
static const SlotSymbol BLUEBERRY_SYMBOL = {
  BLUEBERRY_IMAGE,
  10,
  1
};
static const SlotSymbol CHERRY_SYMBOL = {
  CHERRY_IMAGE,
  10,
  1
};
static const SlotSymbol GRAPE_SYMBOL = {
  GRAPE_IMAGE,
  10,
  1
};
static const SlotSymbol HAZELNUT_SYMBOL = {
  HAZELNUT_IMAGE,
  10,
  1
};
static const SlotSymbol LIME_SYMBOL = {
  LIME_IMAGE,
  10,
  1
};
static const SlotSymbol MANGOSTEEN_SYMBOL = {
  MANGOSTEEN_IMAGE,
  10,
  1
};
static const SlotSymbol ORANGE_SYMBOl = {
  ORANGE_IMAGE,
  10,
  1
};
static const SlotSymbol PEACH_SYMBOL = {
  PEACH_IMAGE,
  10,
  1
};
static const SlotSymbol SCATTER_SYMBOL = {
  SCATTER_IMAGE,
  10,
  1
};
static const SlotSymbol STRAWBERRY_SYMBOL = {
  STRAWBERRY_IMAGE,
  10,
  1
};
static const SlotSymbol WATERMELON_SYMBOL = {
  WATERMELON_IMAGE,
  10,
  1
};
static const SlotSymbol WILD_SYMBOL = {
  WILD_IMAGE,
  10,
  1
};

static const SlotSymbol slotSymbols[] = { APPLE_SYMBOL, BANANA_SYMBOL, BLUEBERRY_SYMBOL, CHERRY_SYMBOL, GRAPE_SYMBOL, HAZELNUT_SYMBOL, LIME_SYMBOL, MANGOSTEEN_SYMBOL, ORANGE_SYMBOl, PEACH_SYMBOL, SCATTER_SYMBOL, STRAWBERRY_SYMBOL, WATERMELON_SYMBOL, WILD_SYMBOL };

static const uint8_t slotSymbolsLength = sizeof(slotSymbols) / sizeof(slotSymbols[0]);

#endif