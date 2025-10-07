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
      if ((currentState == WAITING || currentState == SHUFFLE) || (slotCols[indexStart].frameBuffer.readRow == 0)) {
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
    uint32_t advanceDelayMicroSec = stateConfig[currentState].advanceMicroSecLow;
    if (stateConfig[currentState].randomAdvanceMicroSecMod > 0) {
      advanceDelayMicroSec = (stateConfig[currentState].advanceMicroSecLow +
                        (HAL_RNG_GetRandomNumber(&hrng) %
                         stateConfig[currentState].randomAdvanceMicroSecMod));
    }
    Serial_Debug_Printf("setting timer for %d in state %d\r\n", advanceDelayMicroSec,
                  currentState);
    __HAL_TIM_SET_AUTORELOAD(&NS_TIMER, advanceDelayMicroSec);
    HAL_TIM_Base_Start_IT(&NS_TIMER);
    stateTimerSet = true;
  }
}

void TAO888_SlotMachine_StartCycle() {
  Serial_Debug_Printf("starting cycle\r\n");
  if (currentState == WAITING)
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
    SlotSymbol symbolReciever[SLOT_CELL_COLUMNS * SLOT_CELL_ROWS];
    TAO888_SlotMachine_GetDisplayedSymbols(symbolReciever);
    TAO888_SlotMachine_RoundEndCallback(symbolReciever);
  }
  stateTimerSet = false;
  Serial_Debug_Printf(" -> %d\r\n", currentState);
}

__weak void TAO888_SlotMachine_RoundEndCallback(SlotSymbol* displayedSymbols) {
  // replace with real implementation
  Serial_Debug_Printf("\r\ndisplayed symbol pointer: %x\r\n", displayedSymbols);
  TAO888_SlotMachine_PayoutCallback(20);
}

__weak void TAO888_SlotMachine_PayoutCallback(uint16_t credits) {
  // replace with real implementation
  Serial_Debug_Printf("\r\npaying out: %ld\r\n", credits);
}

SlotSymbol TAO888_SlotMachine_GetRandomSymbol() {
  uint32_t randomNum =
      (HAL_RNG_GetRandomNumber(&hrng) % slotSymbolsRandomUpperBound) + 1;
  uint32_t i = SLOT_SYMBOLS_LENGTH - 1;

  for (; i >=0; i -= 1) {
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