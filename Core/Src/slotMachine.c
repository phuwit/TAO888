#include "slotMachine.h"
#include "banner.h"
#include "main.h"
#include "serialUtils.h"
#include "slotCols.h"
#include "slotSymbols.h"
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_rng.h"
#include "stm32f7xx_hal_tim.h"
#include <stdbool.h>
#include <stdint.h>

extern TIM_HandleTypeDef NS_TIMER;

BannerText bannerTexts[] = {
    {12, 12, "TAO888", &ILI9341_Font_Terminus12x24b, ILI9341_COLOR_BLACK,
     ILI9341_COLOR_WHITE, 0},
    {LCD_WIDTH - 80 - 12, 6, "YOU WIN!", &ILI9341_Font_Terminus10x18b,
     ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE, 0},
    {LCD_WIDTH - 90 - 12, 24, "24 Tokens", &ILI9341_Font_Terminus10x18,
     ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE, 0}};
Banner banner = {bannerTexts, sizeof(bannerTexts) / sizeof(bannerTexts[0])};

static uint32_t slotSymbolsRandomLowerBound[SLOT_SYMBOLS_LENGTH];
static uint32_t slotSymbolsRandomUpperBound;

uint32_t lastUpdate;

SlotCol slotCols[SLOT_CELL_COLUMNS];
SlotSymbol symbolReciever[SLOT_CELL_COLUMNS * SLOT_CELL_ROWS];

volatile State currentState = WAITING;
volatile bool startAdvancingState = false;
volatile bool stateTimerSet = false;

void TAO888_SlotMachine_Init(ILI9341_HandleTypeDef *lcd) {
  for (int i = 0; i < SLOT_SYMBOLS_LENGTH; i += 1) {
    if (i == 0) {
      slotSymbolsRandomLowerBound[0] = 0;
      continue;
    }
    slotSymbolsRandomLowerBound[i] =
        slotSymbolsRandomLowerBound[i - 1] + slotSymbols[i - 1].weight;
  }
  slotSymbolsRandomUpperBound =
      slotSymbolsRandomLowerBound[SLOT_SYMBOLS_LENGTH - 1] +
      slotSymbols[SLOT_SYMBOLS_LENGTH - 1].weight;

  lastUpdate = HAL_GetTick();

  TAO888_Banner_Draw(&banner, lcd);

  // vertical lines
  for (int xOffset = 1; xOffset <= SLOT_CELL_COLUMNS; xOffset++) {
    ILI9341_DrawLine(lcd, (xOffset * SLOT_CELL_SIZE) - 1, BANNER_SIZE,
                     (xOffset * SLOT_CELL_SIZE) - 1, LCD_HEIGHT,
                     ILI9341_COLOR_BLACK);
  }

  TAO888_SlotCols_Init(slotCols, lcd);
  TAO888_SlotCols_CommitAll(slotCols, lcd);
}

void TAO888_SlotMachine_Update(ILI9341_HandleTypeDef *lcd) {
  // switch (currentState) {
  //   case WAITING:
  //     break;
  //   case SHUFFLE:
  //     break;
  //   case SCROLL5:
  //     break;
  //   case SCROLL4:
  //     break;
  //   case SCROLL3:
  //     break;
  //   case SCROLL2:
  //     break;
  //   case SCROLL1:
  //     break;
  //   case RESULT:
  //     break;
  // }
  if (lastUpdate < MINIMUM_UPDATE_MS) {
    return;
  }
  lastUpdate = HAL_GetTick();

  if (stateConfig[currentState].scrollAmount != 0) {
    uint8_t indexStart = stateConfig[currentState].scrollRowStartIndex;
    if (startAdvancingState) {
      if ((currentState == WAITING || currentState == SHUFFLE) ||
          ((slotCols[indexStart].frameBuffer.readRow % SLOT_CELL_SIZE) == 0)) {
        startAdvancingState = false;
        TAO888_SlotMachine_IncrementState();
        return;
      }

      TAO888_SlotCols_ScrollOne(&slotCols[indexStart],
                                SLOT_SCROLL_STOPPING_AMOUNT, true);
      TAO888_SlotCols_CommitOne(&slotCols[indexStart], lcd);
      indexStart += 1;
    }

    for (int index = indexStart;
         index < stateConfig[currentState].scrollRowEndIndex; index += 1) {
      TAO888_SlotCols_ScrollOne(&slotCols[index],
                                stateConfig[currentState].scrollAmount,
                                stateConfig[currentState].scrollSnap);
      TAO888_SlotCols_CommitOne(&slotCols[index], lcd);
    }
  }

  if ((stateConfig[currentState].advanceMicroSecLow != 0) &&
      (stateTimerSet == false)) {
    uint32_t advanceDelayMicroSec =
        stateConfig[currentState].advanceMicroSecLow;
    if (stateConfig[currentState].randomAdvanceMicroSecMod > 0) {
      advanceDelayMicroSec =
          (stateConfig[currentState].advanceMicroSecLow +
           (HAL_RNG_GetRandomNumber(&hrng) %
            stateConfig[currentState].randomAdvanceMicroSecMod));
    }
    if (currentState == RESULT) {
      TAO888_SlotMachine_GetDisplayedSymbols(symbolReciever);
      TAO888_SlotMachine_RoundEndCallback(symbolReciever);
    }
    Serial_Debug_Printf("setting timer for %d in state %d\r\n",
                        advanceDelayMicroSec, currentState);
    __HAL_TIM_SET_AUTORELOAD(&NS_TIMER, advanceDelayMicroSec);
    HAL_TIM_Base_Start_IT(&NS_TIMER);
    stateTimerSet = true;
  }
}

void TAO888_SlotMachine_StartCycle() {
  Serial_Debug_Printf("starting cycle\r\n");
  if (currentState == WAITING || currentState == RESULT)
    TAO888_SlotMachine_IncrementState();
}

void TAO888_SlotMachine_AdvanceStateGracefully() {
  Serial_Debug_Printf("advancing gracefully\r\n");
  const StateConfig config = stateConfig[currentState];
  if (config.advanceOnInput == true) {
    TAO888_SlotMachine_IncrementState();
  } else {
    startAdvancingState = true;
  }
}

void TAO888_SlotMachine_IncrementState() {
  Serial_Debug_Printf("incrementing stage: %d", currentState);
  currentState += 1;
  if (currentState >= (sizeof(stateConfig) / sizeof(stateConfig[0]))) {
    currentState = 0;
    TAO888_SlotCols_Offset(slotCols);
  }
  stateTimerSet = false;
  Serial_Debug_Printf(" -> %d\r\n", currentState);
}

__weak void TAO888_SlotMachine_RoundEndCallback(SlotSymbol *displayedSymbols) {
  uint16_t totalCredits = 0;

  // -------------------- HORIZONTAL --------------------
  for (int r = 0; r < SLOT_CELL_ROWS; r++) {
    int count = 1;
    SlotSymbol *current = &displayedSymbols[r * SLOT_CELL_COLUMNS];
    for (int c = 1; c < SLOT_CELL_COLUMNS; c++) {
      SlotSymbol *next = &displayedSymbols[r * SLOT_CELL_COLUMNS + c];

      if (next->index == WILD_SYMBOL.index ||
          current->index == WILD_SYMBOL.index ||
          next->index == current->index) {
        count++;
        if (current->index == WILD_SYMBOL.index &&
            next->index != WILD_SYMBOL.index)
          current = next;
      } else {
        if (count >= 3) {
          if (current->index == WILD_SYMBOL.index)
            totalCredits += count * 10 * 10;
          else
            totalCredits += count * current->prizeMultiplier * 10;
        }
        count = 1;
        current = next;
      }
    }
    if (count >= 3) {
      if (current->index == WILD_SYMBOL.index)
        totalCredits += count * 10 * 10;
      else
        totalCredits += count * current->prizeMultiplier * 10;
    }
  }

  // -------------------- VERTICAL --------------------
  for (int c = 0; c < SLOT_CELL_COLUMNS; c++) {
    int count = 1;
    SlotSymbol *current = &displayedSymbols[c]; // first row in column
    for (int r = 1; r < SLOT_CELL_ROWS; r++) {
      SlotSymbol *next = &displayedSymbols[r * SLOT_CELL_COLUMNS + c];

      if (next->index == WILD_SYMBOL.index ||
          current->index == WILD_SYMBOL.index ||
          next->index == current->index) {
        count++;
        if (current->index == WILD_SYMBOL.index &&
            next->index != WILD_SYMBOL.index)
          current = next;
      } else {
        if (count >= 3) {
          if (current->index == WILD_SYMBOL.index)
            totalCredits += count * 10 * 10;
          else
            totalCredits += count * current->prizeMultiplier * 10;
        }
        count = 1;
        current = next;
      }
    }
    if (count >= 3) {
      if (current->index == WILD_SYMBOL.index)
        totalCredits += count * 10 * 10;
      else
        totalCredits += count * current->prizeMultiplier * 10;
    }
  }

  // -------------------- DIAGONAL ↘ --------------------
  int countDiag1 = 1;
  SlotSymbol *diag1 = &displayedSymbols[0];
  for (int i = 1; i < SLOT_CELL_ROWS && i < SLOT_CELL_COLUMNS; i++) {
    SlotSymbol *next = &displayedSymbols[i * SLOT_CELL_COLUMNS + i];
    if (next->index == WILD_SYMBOL.index || diag1->index == WILD_SYMBOL.index ||
        next->index == diag1->index) {
      countDiag1++;
      if (diag1->index == WILD_SYMBOL.index && next->index != WILD_SYMBOL.index)
        diag1 = next;
    } else
      break;
  }
  if (countDiag1 >= 3) {
    if (diag1->index == WILD_SYMBOL.index)
      totalCredits += countDiag1 * 10 * 10;
    else
      totalCredits += countDiag1 * diag1->prizeMultiplier * 10;
  }

  // -------------------- DIAGONAL ↗ --------------------
  int countDiag2 = 1;
  SlotSymbol *diag2 =
      &displayedSymbols[(SLOT_CELL_ROWS - 1) * SLOT_CELL_COLUMNS];
  for (int i = 1; i < SLOT_CELL_ROWS && i < SLOT_CELL_COLUMNS; i++) {
    SlotSymbol *next =
        &displayedSymbols[(SLOT_CELL_ROWS - 1 - i) * SLOT_CELL_COLUMNS + i];
    if (next->index == WILD_SYMBOL.index || diag2->index == WILD_SYMBOL.index ||
        next->index == diag2->index) {
      countDiag2++;
      if (diag2->index == WILD_SYMBOL.index && next->index != WILD_SYMBOL.index)
        diag2 = next;
    } else
      break;
  }
  if (countDiag2 >= 3) {
    if (diag2->index == WILD_SYMBOL.index)
      totalCredits += countDiag2 * 10 * 10;
    else
      totalCredits += countDiag2 * diag2->prizeMultiplier * 10;
  }

  // -------------------- SCATTER --------------------
  int scatterCount = 0;
  for (int i = 0; i < SLOT_CELL_ROWS * SLOT_CELL_COLUMNS; i++) {
    if (displayedSymbols[i].index == SCATTER_SYMBOL.index)
      scatterCount++;
  }
  if (scatterCount >= 3)
    totalCredits += scatterCount * 50;

  // -------------------- PAYOUT --------------------
  TAO888_SlotMachine_PayoutCallback(totalCredits);
}

__weak void TAO888_SlotMachine_PayoutCallback(uint16_t credits) {
  Serial_Debug_Printf("\r\nPaying out: %d credits\r\n", credits);
}

SlotSymbol TAO888_SlotMachine_GetRandomSymbol() {
  uint32_t randomNum =
      (HAL_RNG_GetRandomNumber(&hrng) % slotSymbolsRandomUpperBound) + 1;
  uint32_t i = SLOT_SYMBOLS_LENGTH - 1;

  for (; i >= 0; i -= 1) {
    if (randomNum >= slotSymbolsRandomLowerBound[i])
      break;
  }
  return slotSymbols[i];
}

void TAO888_SlotMachine_GetDisplayedSymbols(SlotSymbol *symbolReciever) {
  for (uint8_t col = 0; col < SLOT_CELL_COLUMNS; col += 1) {
    for (uint8_t row = 0; row < SLOT_CELL_ROWS; row += 1) {
      symbolReciever[(col * SLOT_CELL_ROWS) + row] =
          TAO888_SlotColQueue_ReadIndex(&slotCols[col], row + 1);
    }
  }
}