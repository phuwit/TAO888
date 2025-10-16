#include "slotMachine.h"
#include "banner.h"
#include "main.h"
#include "serialUtils.h"
#include "slotCols.h"
#include "slotSymbols.h"
#include "stm32f767xx.h"
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_rng.h"
#include "stm32f7xx_hal_tim.h"
#include "stm32f7xx_hal_uart.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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
int16_t encoderLastValue = 0;

volatile uint32_t credits = 0;
volatile State currentState = WAITING;
volatile bool startAdvancingState = false;
volatile bool stateTimerSet = false;

volatile bool bannerUpdated = false;

bool winLose = false;

uint8_t multiCredits = 10;

void TAO888_SlotMachine_Init(ILI9341_HandleTypeDef *lcd) {
  TAO888_SlotMachine_SendCommandToAux(&AUX_MUSIC_UART_HANDLE, MUSIC_COMMAND_MUSIC_IDLE);

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
	State state = currentState;
	StateConfig config = stateConfig[state];

  if (lastUpdate < MINIMUM_UPDATE_MS) {
    return;
  }
  lastUpdate = HAL_GetTick();

  if (bannerUpdated) {
    bannerUpdated = false;
    TAO888_Banner_Draw(&banner, lcd);
  }

  if (config.scrollAmount != 0) {
    uint8_t indexStart = config.scrollRowStartIndex;
    if (startAdvancingState) {
      if ((state == WAITING || state == SHUFFLE) ||
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
         index < config.scrollRowEndIndex; index += 1) {
      TAO888_SlotCols_ScrollOne(&slotCols[index],
                                config.scrollAmount,
                                config.scrollSnap);
      TAO888_SlotCols_CommitOne(&slotCols[index], lcd);
    }
  }

  if ((config.advanceMicroSecLow != 0) &&
      (stateTimerSet == false) &&
      currentState != WAITING) {
    uint32_t advanceDelayMicroSec =
        config.advanceMicroSecLow;
    if (config.randomAdvanceMicroSecMod > 0) {
      advanceDelayMicroSec =
          (config.advanceMicroSecLow +
           (HAL_RNG_GetRandomNumber(&hrng) %
            config.randomAdvanceMicroSecMod));
    }
    Serial_Debug_Printf("setting timer for %d in state %d\r\n",
                        advanceDelayMicroSec, state);
    __HAL_TIM_SET_AUTORELOAD(&SLOT_STATE_TRANSITION_NS_TIMER, advanceDelayMicroSec);
    HAL_TIM_Base_Start_IT(&SLOT_STATE_TRANSITION_NS_TIMER);
    stateTimerSet = true;
  }
}

void TAO888_SlotMachine_StartCycle() {
  if ((currentState == WAITING) && (credits >= 10)) {
    Serial_Debug_Printf("starting cycle\r\n");
    credits -= 10;
    currentState = SHUFFLE;
    bannerUpdated = true;
    TAO888_Banner_UpdateCredits(&banner, credits);
    TAO888_SlotMachine_SendCommandToAux(&AUX_MUSIC_UART_HANDLE, MUSIC_COMMAND_MUSIC_SPIN);
  } else {
    Serial_Debug_Printf("cannot start cycle\r\n");
    TAO888_SlotMachine_SendCommandToAux(&AUX_MUSIC_UART_HANDLE, MUSIC_COMMAND_MUSIC_LOSE);
    __HAL_TIM_SET_AUTORELOAD(&SLOT_MUSIC_RECOVERY_NS_TIMER, MUSIC_RECOVERY_NS);
    HAL_TIM_Base_Start_IT(&SLOT_MUSIC_RECOVERY_NS_TIMER);
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
  if (currentState == WAITING) {
    TAO888_SlotMachine_SendCommandToAux(&AUX_MUSIC_UART_HANDLE, MUSIC_COMMAND_MUSIC_IDLE);
  }
  stateTimerSet = false;
  Serial_Debug_Printf(" -> %d\r\n", currentState);
	if (currentState == RESULT) {
		TAO888_SlotMachine_GetDisplayedSymbols(symbolReciever);
		TAO888_SlotMachine_RoundEndCallback(symbolReciever);
	}
}

// -------------------- HELPER --------------------
static inline uint16_t scoreLine(SlotSymbol *symbol, int count) {
    float multiplier = (symbol->index == WILD_SYMBOL.index)
        ? WILD_SYMBOL.prizeMultiplier
        : symbol->prizeMultiplier;

    float rawScore = count * multiplier * multiCredits;

    return (uint16_t)(rawScore + 0.5f);
}

// check a custom payline defined by path
static uint16_t checkCustomPayline(SlotSymbol *displayedSymbols, const int path[][2], int pathLength, const char* lineName) {
    Serial_Debug_Printf("%s: [", lineName);
    for (int i = 0; i < pathLength; i++) {
        int col = path[i][0];
        int row = path[i][1];
        if (i > 0) Serial_Debug_Printf(", ");
        Serial_Debug_Printf("%d", displayedSymbols[col * SLOT_CELL_ROWS + row].index);
    }
    Serial_Debug_Printf("] -> ");

    int count = 1;
    SlotSymbol *matchSymbol = &displayedSymbols[path[0][0] * SLOT_CELL_ROWS + path[0][1]];

    // if first position is Wild, find first non-Wild symbol
    if (matchSymbol->index == WILD_SYMBOL.index) {
        for (int i = 1; i < pathLength; i++) {
            int col = path[i][0];
            int row = path[i][1];
            SlotSymbol *candidate = &displayedSymbols[col * SLOT_CELL_ROWS + row];
            if (candidate->index != WILD_SYMBOL.index) {
                matchSymbol = candidate;
                break;
            }
        }
    }

    for (int i = 1; i < pathLength; i++) {
        int col = path[i][0];
        int row = path[i][1];
        SlotSymbol *next = &displayedSymbols[col * SLOT_CELL_ROWS + row];

        if (next->index == WILD_SYMBOL.index || next->index == matchSymbol->index) {
            count++;
        } else {
            break;
        }
    }

    if (count >= 3) {
        uint16_t lineScore = scoreLine(matchSymbol, count);
        Serial_Debug_Printf("Matched %d symbols (idx=%d) => +%d credits\r\n", count, matchSymbol->index, lineScore);
        winLose = true;
        return lineScore;
    } else {
        Serial_Debug_Printf("Not match\r\n");
        return 0;
    }
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
        winLose = true;
        } else {
            Serial_Debug_Printf("Not Match\r\n");
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
            Serial_Debug_Printf("Matched %d symbols (idx=%d) => +%d credits\r\n", count, matchSymbol->index, lineScore);
            winLose = true;
        } else {
            Serial_Debug_Printf("Not Match\r\n");
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
        Serial_Debug_Printf("Matched %d symbols (idx=%d) => +%d credits\r\n", countDiag1, diag1->index, lineScore);
        winLose = true;
    } else {
        Serial_Debug_Printf("Not Match\r\n");
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
        Serial_Debug_Printf("✓ Matched %d symbols (idx=%d) => +%d credits\r\n", countDiag2, diag2->index, lineScore);
        winLose = true;
    } else {
        Serial_Debug_Printf("Not Match\r\n");
    }
    Serial_Debug_Printf("\r\n");

    // -------------------- V-SHAPE PAYLINES --------------------
    if (SLOT_CELL_COLUMNS >= 5 && SLOT_CELL_ROWS >= 3) {
        Serial_Debug_Printf("--- Checking V-SHAPE Paylines ---\r\n");

        // V-Shape ∨
        const int vPath[5][2] = {{0,0}, {1,1}, {2,2}, {3,1}, {4,0}};
        totalCredits += checkCustomPayline(displayedSymbols, vPath, 5, "V-Down  ");

        // Inverted V-Shape ∧
        const int invVPath[5][2] = {{0,2}, {1,1}, {2,0}, {3,1}, {4,2}};
        totalCredits += checkCustomPayline(displayedSymbols, invVPath, 5, "V-Up    ");

        Serial_Debug_Printf("\r\n");
    }

    // -------------------- ZIGZAG PAYLINES --------------------
    if (SLOT_CELL_COLUMNS >= 5 && SLOT_CELL_ROWS >= 3) {
        Serial_Debug_Printf("--- Checking ZIGZAG Paylines ---\r\n");

        // W-Shape (center-top-center-top-center)
        const int wPath[5][2] = {{0,1}, {1,0}, {2,1}, {3,0}, {4,1}};
        totalCredits += checkCustomPayline(displayedSymbols, wPath, 5, "Zigzag W");

        // M-Shape (top-center-top-center-top)
        const int mPath[5][2] = {{0,0}, {1,1}, {2,0}, {3,1}, {4,0}};
        totalCredits += checkCustomPayline(displayedSymbols, mPath, 5, "Zigzag M");

        // Zigzag Lower (center-bottom-center-bottom-center)
        const int zLowerPath[5][2] = {{0,1}, {1,2}, {2,1}, {3,2}, {4,1}};
        totalCredits += checkCustomPayline(displayedSymbols, zLowerPath, 5, "Zigzag L");

        // M-Lower (bottom-center-bottom-center-bottom)
        const int mLowerPath[5][2] = {{0,2}, {1,1}, {2,2}, {3,1}, {4,2}};
        totalCredits += checkCustomPayline(displayedSymbols, mLowerPath, 5, "Zigzag m");

        Serial_Debug_Printf("\r\n");
    }

    // -------------------- STEP PATTERN PAYLINES --------------------
    if (SLOT_CELL_COLUMNS >= 5 && SLOT_CELL_ROWS >= 3) {
        Serial_Debug_Printf("--- Checking STEP Paylines ---\r\n");

        // step up 1: bottom-bottom-center-center-top
        const int stepUp1[5][2] = {{0,2}, {1,2}, {2,1}, {3,1}, {4,0}};
        totalCredits += checkCustomPayline(displayedSymbols, stepUp1, 5, "Step ⌊⌉ ");

        // step up 2: bottom-center-center-top-top
        const int stepUp2[5][2] = {{0,2}, {1,1}, {2,1}, {3,0}, {4,0}};
        totalCredits += checkCustomPayline(displayedSymbols, stepUp2, 5, "Step ⌊‾ ");

        // step down 1: top-top-center-center-bottom
        const int stepDown1[5][2] = {{0,0}, {1,0}, {2,1}, {3,1}, {4,2}};
        totalCredits += checkCustomPayline(displayedSymbols, stepDown1, 5, "Step ⌈⌋ ");

        // step down 2: top-center-center-bottom-bottom
        const int stepDown2[5][2] = {{0,0}, {1,1}, {2,1}, {3,2}, {4,2}};
        totalCredits += checkCustomPayline(displayedSymbols, stepDown2, 5, "Step ‾⌋ ");

        Serial_Debug_Printf("\r\n");
    }

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
        uint16_t scatterScore = scatterCount * SCATTER_SYMBOL.prizeMultiplier;
        totalCredits += scatterScore;
        Serial_Debug_Printf("->%d scatters => +%d credits\r\n", scatterCount, scatterScore);
        winLose = true;
    } else {
        Serial_Debug_Printf("->Not Match\r\n", scatterCount);
    }

    // -------------------- FINAL PAYOUT --------------------
    Serial_Debug_Printf("\r\n========== PAYOUT ==========\r\n");
    Serial_Debug_Printf("TOTAL Credits (before coin payout): %d\r\n", totalCredits);
    Serial_Debug_Printf("============================\r\n\r\n");

    if (totalCredits > 0) {
			int payoutCoins = totalCredits / 10;
			int remainCredits = totalCredits % 10;

			TAO888_SlotMachine_SendCommandToAux(&AUX_MUSIC_UART_HANDLE, MUSIC_COMMAND_MUSIC_WIN);
			Serial_Debug_Printf(
				"Total %d credit(s) → Payout %d credit(s), keep %d credit(s)\r\n",
				totalCredits, payoutCoins, remainCredits
			);

			if (payoutCoins > 0) {
				TAO888_SlotMachine_PayoutCallback(payoutCoins);
			}

			if (remainCredits > 0) {
				TAO888_SlotMachine_IncrementCredits(remainCredits);
			}
    } else {
			TAO888_SlotMachine_SendCommandToAux(&AUX_MUSIC_UART_HANDLE, MUSIC_COMMAND_MUSIC_LOSE);
      Serial_Debug_Printf("You Lose..so sad\r\n");
    }
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
	TAO888_SlotMachine_SendCommandToAux(&AUX_MUSIC_UART_HANDLE, MUSIC_COMMAND_MUSIC_COIN);
  TAO888_Banner_UpdateCredits(&banner, credits);
  bannerUpdated = true;
  Serial_Debug_Printf("credits = %u\r\n", credits);
  __HAL_TIM_SET_AUTORELOAD(&SLOT_MUSIC_RECOVERY_NS_TIMER, MUSIC_RECOVERY_NS);
  HAL_TIM_Base_Start_IT(&SLOT_MUSIC_RECOVERY_NS_TIMER);
}

void TAO888_SlotMachine_SendCommandToAux(UART_HandleTypeDef* AuxUart, const uint8_t command) {
  Serial_Debug_Printf("sending command to aux: %x\r\n", command);
  HAL_UART_Transmit(AuxUart, &command, sizeof(command), AUX_UART_TIMEOUT);
}

void TAO888_SlotMachine_PollRotaryEncoderAndStart(TIM_TypeDef* EncoderHandle) {
  int16_t currentValue = EncoderHandle->CNT;
  // Serial_Debug_Printf("encoder: %d -> %d\r\n", encoderLastValue, currentValue);
  if ((currentValue - encoderLastValue) >= SLOT_ENCODER_START_THRESHOLD) {
    TAO888_SlotMachine_StartCycle();
    encoderLastValue = currentValue;
  } else if ((currentValue - encoderLastValue) < 0) {
    encoderLastValue = currentValue;
  }
}

void TAO888_SlotMachine_ResetCredits() {
  Serial_Debug_Printf("resetting credits\r\n");
  credits = 0;
	TAO888_SlotMachine_SendCommandToAux(&AUX_MUSIC_UART_HANDLE, MUSIC_COMMAND_MUSIC_COIN);
  TAO888_Banner_UpdateCredits(&banner, credits);
  bannerUpdated = true;
  Serial_Debug_Printf("credits = %u\r\n", credits);
}

void TAO888_SlotMachine_RecoverMusic() {
  TAO888_SlotMachine_SendCommandToAux(&AUX_MUSIC_UART_HANDLE, stateConfig[currentState].stateMusicCommand);
}