#ifndef TAO888_SLOT_COLS
#define TAO888_SLOT_COLS

#include "frameBuffer.h"
#include "ili9341.h"
#include "queue.h"
#include "slotSymbols.h"
#include <stdint.h>

#define TOP_PADDING_CELLS 1
#define WRITABLE_COL_CELLS 3
#define SYMBOL_QUEUE_SIZE (TOP_PADDING_CELLS + WRITABLE_COL_CELLS)

typedef struct {
  FrameBuffer frameBuffer;
  SlotSymbol symbolQueue[SYMBOL_QUEUE_SIZE];
  uint8_t symbolQueueTailIndex;
  uint8_t symbolQueueUsedSize;
} SlotCol;

void TAO888_SlotCols_Init(SlotCol *slotCols, ILI9341_HandleTypeDef *lcd);
void TAO888_SlotCols_Offset(SlotCol *slotCols);
void TAO888_SlotCols_Commit(SlotCol *slotCols, ILI9341_HandleTypeDef *lcd);
void TAO888_SlotCols_Scroll(SlotCol *slotCols, int8_t scrollAmount);

#endif