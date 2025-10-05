#ifndef TAO888_SLOT_MACHINE
#define TAO888_SLOT_MACHINE

#include <stdbool.h>
#include <stdint.h>

#include "ili9341.h"

typedef enum {
  WAITING,
  SHUFFLE,
  SCROLL5,
  SCROLL4,
  SCROLL3,
  SCROLL2,
  SCROLL1,
  RESULT,
} State;

typedef struct {
  bool autoAdvance;
  bool randomAdvance;
  uint16_t randomAdvanceMsLow;
  uint16_t randomAdvanceMsHigh;
  bool scroll;
  int8_t scrollAmount;
  uint8_t scrollRowStartIndex;
  uint8_t scrollRowEndIndex;
  bool scrollSnap;
} StateConfig;

static const StateConfig stateConfig[] = {
    {false, false, 0, 0, true, -16, 0, 5, false},
    {true, true, 1000, 2000, true, -24, 0, 5, false},
    {true, true, 1000, 2000, true, -8, 0, 5, true},
    {true, true, 1000, 2000, true, -8, 1, 5, true},
    {true, true, 1000, 2000, true, -8, 2, 5, true},
    {true, true, 1000, 2000, true, -8, 3, 5, true},
    {true, true, 1000, 2000, true, -8, 4, 5, true},
    {true, false, 5000, 5000, false, 0, 0, 0, true},
};

void TAO888_SlotMachine_Init(ILI9341_HandleTypeDef *lcd);
void TAO888_SlotMachine_Update(ILI9341_HandleTypeDef *lcd);
void TAO888_SlotMachine_StartCycle();
void TAO888_SlotMachine_IncrementState();

#endif