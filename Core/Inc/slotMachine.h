#ifndef TAO888_SLOT_MACHINE
#define TAO888_SLOT_MACHINE

#include <stdbool.h>
#include <stdint.h>

#include "ili9341.h"

enum State {
  WAITING,
  SHUFFLE,
  SCROLL5,
  SCROLL4,
  SCROLL3,
  SCROLL2,
  SCROLL1,
  RESULT,
};

typedef struct {
  bool scroll;
  int8_t scrollAmount;
  uint8_t scrollRowStartIndex;
  uint8_t scrollRowEndIndex;
} StateConfig;

static const StateConfig stateConfig[] = {
  {true, -24, 0, 4},
  {true, -16, 0, 4},
  {true, -8, 0, 4},
  {true, -8, 1, 4},
  {true, -8, 2, 4},
  {true, -8, 3, 4},
  {true, -8, 4, 4},
  {false, 0, 0, 0},
};

void TAO888_SlotMachine_Init(ILI9341_HandleTypeDef* lcd);
void TAO888_SlotMachine_Update(ILI9341_HandleTypeDef* lcd);

#endif