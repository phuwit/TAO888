#include "slotMachine.h"
#include "banner.h"
#include "main.h"
#include "serialUtils.h"
#include "slotCols.h"
#include "slotSymbols.h"
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_rng.h"
#include "stm32f7xx_hal_tim.h"
#include "stm32f7xx_hal_uart.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern TIM_HandleTypeDef NS_TIMER;

BannerText bannerTexts[] = {
    {12, 12, "TAO888", &ILI9341_Font_Terminus12x24b, ILI9341_COLOR_BLACK,
     ILI9341_COLOR_WHITE, 0},
    {LCD_WIDTH - 110 - 12, 6, "    Welcome", &ILI9341_Font_Terminus10x18b,
     ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE, 0},
    {LCD_WIDTH - 90 - 12, 24, "0 Credits", &ILI9341_Font_Terminus10x18,
     ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE, 0}};
Banner banner = {bannerTexts, sizeof(bannerTexts) / sizeof(bannerTexts[0])};

static uint32_t slotSymbolsRandomLowerBound[SLOT_SYMBOLS_LENGTH];
static uint32_t slotSymbolsRandomUpperBound;

uint32_t lastUpdate;

SlotCol slotCols[SLOT_CELL_COLUMNS];
SlotSymbol symbolReciever[SLOT_CELL_COLUMNS * SLOT_CELL_ROWS];

uint8_t payoutAmount = 0;

volatile uint32_t credits = 0;
volatile State currentState = WAITING;
volatile bool startAdvancingState = false;
volatile bool stateTimerSet = false;
volatile bool bannerUpdated = false;

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

  if (bannerUpdated) {
    bannerUpdated = false;
    TAO888_Banner_Draw(&banner, lcd);
  }

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
  if ((currentState == WAITING || currentState == RESULT) && (credits > 0)) {
    Serial_Debug_Printf("starting cycle\r\n");
    credits -= 1;
    currentState = SHUFFLE;
    bannerUpdated = true;
    TAO888_Banner_UpdateCredits(&banner, credits);
  } else {
    Serial_Debug_Printf("cannot start cycle\r\n");
  }
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

// -------------------- HELPER --------------------
static inline uint16_t scoreLine(SlotSymbol *symbol, int count) {
    if (symbol->index == WILD_SYMBOL.index)
        return count * 10 * 10; // all wilds
    return count * symbol->prizeMultiplier * 10;
}

__weak void TAO888_SlotMachine_RoundEndCallback(SlotSymbol *displayedSymbols) {
    uint16_t totalCredits = 0;

    // Column-Major: [Col0][Col1][Col2]...
    // index = c * SLOT_CELL_ROWS + r

    // -------------------- DEBUG: display Slot --------------------
    Serial_Debug_Printf("\r\n========== SLOT RESULT ==========\r\n");
    Serial_Debug_Printf("\r\n");

    Serial_Debug_Printf("     |");
    for (int c = 0; c < SLOT_CELL_COLUMNS; c++) {
        Serial_Debug_Printf(" Col%-2d |", c);
    }
    Serial_Debug_Printf("\r\n");

    Serial_Debug_Printf("-----|");
    for (int c = 0; c < SLOT_CELL_COLUMNS; c++) {
        Serial_Debug_Printf("-------|");
    }
    Serial_Debug_Printf("\r\n");

    // each row (Column-Major indexing)
    for (int r = 0; r < SLOT_CELL_ROWS; r++) {
        Serial_Debug_Printf("Row%-2d|", r);
        for (int c = 0; c < SLOT_CELL_COLUMNS; c++) {
            int idx = c * SLOT_CELL_ROWS + r;  // Column-Major!
            SlotSymbol *sym = &displayedSymbols[idx];
            Serial_Debug_Printf("  %-4d |", sym->index);
        }
        Serial_Debug_Printf("\r\n");
    }
    Serial_Debug_Printf("\r\n\r\n");

    // -------------------- HORIZONTAL PAYLINES (Left-to-Right) --------------------
    Serial_Debug_Printf("--- Checking HORIZONTAL Paylines ---\r\n");
    for (int r = 0; r < SLOT_CELL_ROWS; r++) {
        int count = 1;
        SlotSymbol *matchSymbol = &displayedSymbols[r];  // Column 0, Row r

        // display symbols in this row
        Serial_Debug_Printf("Row %d: [", r);
        for (int c = 0; c < SLOT_CELL_COLUMNS; c++) {
            if (c > 0) Serial_Debug_Printf(", ");
            Serial_Debug_Printf("%d", displayedSymbols[c * SLOT_CELL_ROWS + r].index);
        }
        Serial_Debug_Printf("] -> ");

        // if first position is Wild, find first non-Wild symbol
        if (matchSymbol->index == WILD_SYMBOL.index) {
            for (int c = 1; c < SLOT_CELL_COLUMNS; c++) {
                SlotSymbol *candidate = &displayedSymbols[c * SLOT_CELL_ROWS + r];
                if (candidate->index != WILD_SYMBOL.index) {
                    matchSymbol = candidate;
                    break;
                }
            }
        }

        // check each position from left to right
        for (int c = 1; c < SLOT_CELL_COLUMNS; c++) {
            SlotSymbol *next = &displayedSymbols[c * SLOT_CELL_ROWS + r];
            if (next->index == WILD_SYMBOL.index || next->index == matchSymbol->index) {
                count++;
            } else {
                break;
            }
        }

        if (count >= 3) {
            uint16_t lineScore = scoreLine(matchSymbol, count);
            totalCredits += lineScore;
            Serial_Debug_Printf("✓ Matched %d symbols (idx=%d) => +%d credits\r\n",
                                count, matchSymbol->index, lineScore);
        } else {
            Serial_Debug_Printf("✗ Only %d symbols (need 3+)\r\n", count);
        }
    }
    Serial_Debug_Printf("\r\n");

    // -------------------- VERTICAL PAYLINES (Top-to-Bottom) --------------------
    Serial_Debug_Printf("--- Checking VERTICAL Paylines ---\r\n");
    for (int c = 0; c < SLOT_CELL_COLUMNS; c++) {
        int count = 1;
        SlotSymbol *matchSymbol = &displayedSymbols[c * SLOT_CELL_ROWS];  // Column c, Row 0

        // display symbols in this column
        Serial_Debug_Printf("Col %d: [", c);
        for (int r = 0; r < SLOT_CELL_ROWS; r++) {
            if (r > 0) Serial_Debug_Printf(", ");
            Serial_Debug_Printf("%d", displayedSymbols[c * SLOT_CELL_ROWS + r].index);
        }
        Serial_Debug_Printf("] -> ");

        // if first position is Wild, find first non-Wild symbol
        if (matchSymbol->index == WILD_SYMBOL.index) {
            for (int r = 1; r < SLOT_CELL_ROWS; r++) {
                SlotSymbol *candidate = &displayedSymbols[c * SLOT_CELL_ROWS + r];
                if (candidate->index != WILD_SYMBOL.index) {
                    matchSymbol = candidate;
                    break;
                }
            }
        }

        // check each position from top to bottom
        for (int r = 1; r < SLOT_CELL_ROWS; r++) {
            SlotSymbol *next = &displayedSymbols[c * SLOT_CELL_ROWS + r];
            if (next->index == WILD_SYMBOL.index || next->index == matchSymbol->index) {
                count++;
            } else {
                break;
            }
        }

        if (count >= 3) {
            uint16_t lineScore = scoreLine(matchSymbol, count);
            totalCredits += lineScore;
            Serial_Debug_Printf("✓ Matched %d symbols (idx=%d) => +%d credits\r\n",
                                 count, matchSymbol->index, lineScore);
        } else {
            Serial_Debug_Printf("✗ Only %d symbols (need 3+)\r\n", count);
        }
    }
    Serial_Debug_Printf("\r\n");

    // -------------------- DOWN DIAGONAL (Top-Left to Bottom-Right) --------------------
    Serial_Debug_Printf("--- Checking DOWN DIAGONAL ---\r\n");
    int minDim = (SLOT_CELL_ROWS < SLOT_CELL_COLUMNS) ? SLOT_CELL_ROWS : SLOT_CELL_COLUMNS;
    int countDiag1 = 1;
    SlotSymbol *diag1 = &displayedSymbols[0];  // (0,0)

    Serial_Debug_Printf("Down Diag: [");
    for (int i = 0; i < minDim; i++) {
        if (i > 0) Serial_Debug_Printf(", ");
        Serial_Debug_Printf("%d", displayedSymbols[i * SLOT_CELL_ROWS + i].index);
    }
    Serial_Debug_Printf("] -> ");

    if (diag1->index == WILD_SYMBOL.index) {
        for (int i = 1; i < minDim; i++) {
            SlotSymbol *candidate = &displayedSymbols[i * SLOT_CELL_ROWS + i];
            if (candidate->index != WILD_SYMBOL.index) {
                diag1 = candidate;
                break;
            }
        }
    }

    for (int i = 1; i < minDim; i++) {
        SlotSymbol *next = &displayedSymbols[i * SLOT_CELL_ROWS + i];
        if (next->index == WILD_SYMBOL.index || next->index == diag1->index) {
            countDiag1++;
        } else {
            break;
        }
    }

    if (countDiag1 >= 3) {
        uint16_t lineScore = scoreLine(diag1, countDiag1);
        totalCredits += lineScore;
        Serial_Debug_Printf("✓ Matched %d symbols (idx=%d) => +%d credits\r\n",
                             countDiag1, diag1->index, lineScore);
    } else {
        Serial_Debug_Printf("✗ Only %d symbols (need 3+)\r\n", countDiag1);
    }

    // -------------------- UP DIAGONAL (Bottom-Left to Top-Right) --------------------
    Serial_Debug_Printf("--- Checking UP DIAGONAL ---\r\n");
    int countDiag2 = 1;
    SlotSymbol *diag2 = &displayedSymbols[SLOT_CELL_ROWS - 1];  // (0, ROWS-1)

    Serial_Debug_Printf("Up Diag : [");
    for (int i = 0; i < minDim; i++) {
        if (i > 0) Serial_Debug_Printf(", ");
        Serial_Debug_Printf("%d", displayedSymbols[i * SLOT_CELL_ROWS + (SLOT_CELL_ROWS - 1 - i)].index);
    }
    Serial_Debug_Printf("] -> ");

    if (diag2->index == WILD_SYMBOL.index) {
        for (int i = 1; i < minDim; i++) {
            SlotSymbol *candidate = &displayedSymbols[i * SLOT_CELL_ROWS + (SLOT_CELL_ROWS - 1 - i)];
            if (candidate->index != WILD_SYMBOL.index) {
                diag2 = candidate;
                break;
            }
        }
    }

    for (int i = 1; i < minDim; i++) {
        SlotSymbol *next = &displayedSymbols[i * SLOT_CELL_ROWS + (SLOT_CELL_ROWS - 1 - i)];
        if (next->index == WILD_SYMBOL.index || next->index == diag2->index) {
            countDiag2++;
        } else {
            break;
        }
    }

    if (countDiag2 >= 3) {
        uint16_t lineScore = scoreLine(diag2, countDiag2);
        totalCredits += lineScore;
        Serial_Debug_Printf("✓ Matched %d symbols (idx=%d) => +%d credits\r\n",
                             countDiag2, diag2->index, lineScore);
    } else {
        Serial_Debug_Printf("✗ Only %d symbols (need 3+)\r\n", countDiag2);
    }
    Serial_Debug_Printf("\r\n");

    // -------------------- SCATTER BONUS --------------------
    Serial_Debug_Printf("--- Checking SCATTER ---\r\n");
    int scatterCount = 0;
    Serial_Debug_Printf("Scatter positions: ");
    for (int i = 0; i < SLOT_CELL_ROWS * SLOT_CELL_COLUMNS; i++) {
        if (displayedSymbols[i].index == SCATTER_SYMBOL.index) {
            scatterCount++;
            // change to (row, col)
            int col = i / SLOT_CELL_ROWS;
            int row = i % SLOT_CELL_ROWS;
            Serial_Debug_Printf("(R%d,C%d) ", row, col);
        }
    }

    if (scatterCount >= 3) {
        uint16_t scatterScore = scatterCount * 50;
        totalCredits += scatterScore;
        Serial_Debug_Printf("-> ✓ %d scatters => +%d credits\r\n", scatterCount, scatterScore);
    } else {
        Serial_Debug_Printf("-> ✗ Only %d scatters (need 3+)\r\n", scatterCount);
    }

    // -------------------- FINAL PAYOUT --------------------
    Serial_Debug_Printf("\r\n========== PAYOUT ==========\r\n");
    Serial_Debug_Printf("TOTAL Credits: %d\r\n", totalCredits);
    Serial_Debug_Printf("============================\r\n\r\n");
    TAO888_SlotMachine_PayoutCallback(totalCredits);
  }

__weak void TAO888_SlotMachine_PayoutCallback(uint16_t coinAmount) {
  payoutAmount = coinAmount;
  Serial_Debug_Printf("\r\nPaying out: %d credits\r\n", payoutAmount);
  TAO888_SlotMachine_SendCommandToAux(&AUX_COIN_UART_HANDLE, payoutAmount);
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

void TAO888_SlotMachine_IncrementCredits(uint8_t amount) {
  Serial_Debug_Printf("adding credits\r\n");

  credits += amount;
  TAO888_Banner_UpdateCredits(&banner, credits);
  bannerUpdated = true;
  Serial_Debug_Printf("credits = %u\r\n", credits);

}

void TAO888_SlotMachine_SendCommandToAux(UART_HandleTypeDef* AuxUart, const uint8_t command) {
  HAL_UART_Transmit(AuxUart, &command, sizeof(command), AUX_UART_TIMEOUT);
}