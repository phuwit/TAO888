#ifndef TAO888_SLOTSYMBOLS
#define TAO888_SLOTSYMBOLS

#include "images.h"
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

// -------------------- SYMBOL STRUCT --------------------
typedef struct {
  uint8_t index;
  Image image;
  uint8_t weight;
  float prizeMultiplier;
} SlotSymbol;

// -------------------- SYMBOLS --------------------
// static const SlotSymbol APPLE_SYMBOL       = {0, APPLE_IMAGE,       30, 1 };
static const SlotSymbol BANANA_SYMBOL = {0, BANANA_IMAGE, 15, 0.2};
static const SlotSymbol BLUEBERRY_SYMBOL = {1, BLUEBERRY_IMAGE, 15, 0.2};
static const SlotSymbol CHERRY_SYMBOL = {2, CHERRY_IMAGE, 12, 0.3};
static const SlotSymbol GRAPE_SYMBOL = {3, GRAPE_IMAGE, 12, 0.3};
static const SlotSymbol HAZELNUT_SYMBOL = {4, HAZELNUT_IMAGE, 7, 0.4};
// static const SlotSymbol LIME_SYMBOL        = {6, LIME_IMAGE,        10, 4 };
// static const SlotSymbol MANGOSTEEN_SYMBOL  = {7, MANGOSTEEN_IMAGE,   8, 5 };
// static const SlotSymbol ORANGE_SYMBOL = {8, ORANGE_IMAGE, 8, 5};
static const SlotSymbol PEACH_SYMBOL = {5, PEACH_IMAGE, 7, 0.4};
static const SlotSymbol STRAWBERRY_SYMBOL = {6, STRAWBERRY_IMAGE, 5, 0.5};
static const SlotSymbol WATERMELON_SYMBOL = {7, WATERMELON_IMAGE, 4, 0.6};
static const SlotSymbol WILD_SYMBOL = {8, WILD_IMAGE, 3, 0.8};
static const SlotSymbol SCATTER_SYMBOL = {9, SCATTER_IMAGE, 2, 3};

#define SLOT_SYMBOLS_LENGTH 10

// static const SlotSymbol slotSymbols[] = {
//     APPLE_SYMBOL, BANANA_SYMBOL, BLUEBERRY_SYMBOL, CHERRY_SYMBOL,
//     GRAPE_SYMBOL, HAZELNUT_SYMBOL, LIME_SYMBOL, MANGOSTEEN_SYMBOL,
//     ORANGE_SYMBOL, PEACH_SYMBOL, STRAWBERRY_SYMBOL, WATERMELON_SYMBOL,
//     SCATTER_SYMBOL, WILD_SYMBOL
// };

static const SlotSymbol slotSymbols[] = {
    BANANA_SYMBOL,   BLUEBERRY_SYMBOL, CHERRY_SYMBOL,     GRAPE_SYMBOL,
    HAZELNUT_SYMBOL, PEACH_SYMBOL,     STRAWBERRY_SYMBOL, WATERMELON_SYMBOL,
    WILD_SYMBOL, SCATTER_SYMBOL};

#endif