#include "slotCols.h"

#include "frameBuffer.h"
#include "ili9341.h"
#include "main.h"
#include "slotSymbols.h"
#include "utils.h"
#include <stdint.h>

void TAO888_SlotCols_Enqueue(SlotCol *slotCol, SlotSymbol symbol) {
  slotCol->symbolQueue[slotCol->symbolQueueTailIndex] = symbol;
  slotCol->symbolQueueTailIndex =
      (slotCol->symbolQueueTailIndex + 1) % SYMBOL_QUEUE_SIZE;
}

void TAO888_SlotCols_ReplaceHead(SlotCol *slotCol, SlotSymbol symbol) {
  slotCol->symbolQueueTailIndex =
      (slotCol->symbolQueueTailIndex - 1 + SYMBOL_QUEUE_SIZE) %
      SYMBOL_QUEUE_SIZE;
  slotCol->symbolQueue[slotCol->symbolQueueTailIndex] = symbol;
}

SlotSymbol TAO888_SlotCols_ReadIndex(SlotCol *slotCol, const uint8_t index) {
  const uint8_t readIndex =
      (slotCol->symbolQueueTailIndex + index) % SYMBOL_QUEUE_SIZE;
  return slotCol->symbolQueue[readIndex];
}

void TAO888_SlotCols_Init(SlotCol *slotCols, ILI9341_HandleTypeDef *lcd) {
  for (int col = 0; col < 5; col += 1) {
    slotCols[col].frameBuffer = TAO888_FrameBuffer_Initialize(
        (col * SLOT_CELL_SIZE), BANNER_SIZE, SLOT_CELL_SIZE - 1,
        SLOT_CELL_SIZE * WRITABLE_COL_CELLS, SLOT_CELL_SIZE * TOP_PADDING_CELLS,
        0, 0, 0);
    slotCols[col].symbolQueueTailIndex = 0;
    slotCols[col].symbolQueueUsedSize = 0;
    TAO888_FrameBuffer_Fill(&slotCols[col].frameBuffer, ILI9341_COLOR_WHITE);

    for (int row = 0; row < 4; row++) {
      SlotSymbol symbol = TAO888_Utils_GetRandomSymbol();
      TAO888_SlotCols_Enqueue(&slotCols[col], symbol);

      TAO888_FrameBuffer_DrawImage(
          &slotCols[col].frameBuffer, SLOT_CELL_PADDING_X,
          SLOT_CELL_PADDING_Y + (row * SLOT_CELL_SIZE), symbol.image.width,
          symbol.image.height, symbol.image.data);
    }

    for (int yOffset = 0; yOffset <= 3; yOffset++) {
      TAO888_FrameBuffer_DrawLine(
          &slotCols[col].frameBuffer, 0, yOffset * SLOT_CELL_SIZE, LCD_WIDTH,
          yOffset * SLOT_CELL_SIZE, ILI9341_COLOR_BLACK);
    }
  }

  TAO888_SlotCols_Offset(slotCols);
  TAO888_SlotCols_Commit(slotCols, lcd);
}

void TAO888_SlotCols_Offset(SlotCol *slotCols) {
  for (int i = 0; i < 5; i += 1) {
    TAO888_FrameBuffer_IncrementReadRow(&slotCols[i].frameBuffer,
                                        ((SLOT_CELL_OFFSET_SIZE) * i));
  }
}

void TAO888_SlotCols_Commit(SlotCol *slotCols, ILI9341_HandleTypeDef *lcd) {
  for (int i = 0; i < 5; i += 1) {
    TAO888_FrameBuffer_Commit(&slotCols[i].frameBuffer, lcd);
  }
}

void TAO888_SlotCols_Scroll(SlotCol *slotCols,
                            int8_t scrollAmount) {
  for (int col = 0; col < 5; col += 1) {
    if (TAO888_FrameBuffer_IncrementReadRow(&slotCols[col].frameBuffer,
                                            scrollAmount)) {
      const SlotSymbol newSymbol = TAO888_Utils_GetRandomSymbol();
      TAO888_SlotCols_ReplaceHead(&slotCols[col], newSymbol);

      for (int row = 0; row < 4; row++) {
        const SlotSymbol symbol =
            TAO888_SlotCols_ReadIndex(&slotCols[col], row);
        TAO888_FrameBuffer_DrawImage(
            &slotCols[col].frameBuffer, SLOT_CELL_PADDING_X,
            SLOT_CELL_PADDING_Y + (row * SLOT_CELL_SIZE), symbol.image.width,
            symbol.image.height, symbol.image.data);
      }
    }
  }
}