#ifndef TAO888_SLOTSYMBOLS
#define TAO888_SLOTSYMBOLS

#include "images.h"

typedef struct {
  Image image;
  uint32_t weight;
  uint32_t prizeMultiplier;
} SlotSymbol;

static const SlotSymbol APPLE_SYMBOL = {
  APPLE_IMAGE,
  10000,
  1
};
static const SlotSymbol BANANA_SYMBOL = {
  BANANA_IMAGE,
  9000,
  2
};
static const SlotSymbol BLUEBERRY_SYMBOL = {
  BLUEBERRY_IMAGE,
  8000,
  3
};
static const SlotSymbol CHERRY_SYMBOL = {
  CHERRY_IMAGE,
  7000,
  4
};
static const SlotSymbol GRAPE_SYMBOL = {
  GRAPE_IMAGE,
  6000,
  5
};
static const SlotSymbol HAZELNUT_SYMBOL = {
  HAZELNUT_IMAGE,
  5000,
  10
};
static const SlotSymbol LIME_SYMBOL = {
  LIME_IMAGE,
  4000,
  15
};
static const SlotSymbol MANGOSTEEN_SYMBOL = {
  MANGOSTEEN_IMAGE,
  3000,
  20
};
static const SlotSymbol ORANGE_SYMBOl = {
  ORANGE_IMAGE,
  2000,
  25
};
static const SlotSymbol STRAWBERRY_SYMBOL = {
  STRAWBERRY_IMAGE,
  1000,
  30
};
static const SlotSymbol WATERMELON_SYMBOL = {
  WATERMELON_IMAGE,
  500,
  70
};
static const SlotSymbol PEACH_SYMBOL = {
  PEACH_IMAGE,
  200,
  150
};
static const SlotSymbol SCATTER_SYMBOL = {
  SCATTER_IMAGE,
  80,
  400
};
static const SlotSymbol WILD_SYMBOL = {
  WILD_IMAGE,
  50,
  500
};

#define SLOT_SYMBOLS_LENGTH 14

static const SlotSymbol slotSymbols[SLOT_SYMBOLS_LENGTH] = { APPLE_SYMBOL, BANANA_SYMBOL, BLUEBERRY_SYMBOL, CHERRY_SYMBOL, GRAPE_SYMBOL, HAZELNUT_SYMBOL, LIME_SYMBOL, MANGOSTEEN_SYMBOL, ORANGE_SYMBOl, PEACH_SYMBOL, STRAWBERRY_SYMBOL, WATERMELON_SYMBOL, SCATTER_SYMBOL, WILD_SYMBOL };

#endif