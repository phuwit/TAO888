#include "slotMachine.h"
#include "banner.h"
#include "main.h"
#include "serialUtils.h"
#include "slotCols.h"

BannerText bannerTexts[] = {
    {12, 12, "TAO888", &ILI9341_Font_Terminus12x24b, ILI9341_COLOR_BLACK,
     ILI9341_COLOR_WHITE, 0},
    {LCD_WIDTH - 80 - 12, 6, "YOU WIN!", &ILI9341_Font_Terminus10x18b,
     ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE, 0},
    {LCD_WIDTH - 90 - 12, 24, "24 Tokens", &ILI9341_Font_Terminus10x18,
     ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE, 0}};
Banner banner = {bannerTexts, sizeof(bannerTexts) / sizeof(bannerTexts[0])};

SlotCol slotCols[5];

State currentState = WAITING;
int8_t scrollAmount = -8;

void TAO888_SlotMachine_Init(ILI9341_HandleTypeDef *lcd) {
  TAO888_Banner_Draw(&banner, lcd);

  // vertical lines
  for (int xOffset = 1; xOffset <= 5; xOffset++) {
    ILI9341_DrawLine(lcd, (xOffset * SLOT_CELL_SIZE) - 1, BANNER_SIZE,
                     (xOffset * SLOT_CELL_SIZE) - 1, LCD_HEIGHT,
                     ILI9341_COLOR_BLACK);
  }

  TAO888_SlotCols_Init(slotCols, lcd);
  TAO888_SlotCols_Commit(slotCols, lcd);
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
  const StateConfig config = stateConfig[currentState];
  if (config.scroll) {
    for (int index = config.scrollRowStartIndex; index < config.scrollRowEndIndex; index += 1) {
      TAO888_SlotCols_ScrollOne(&slotCols[index], config.scrollAmount, config.scrollSnap);
    }
    TAO888_SlotCols_Commit(slotCols, lcd);
  }
}

void TAO888_SlotMachine_StartCycle() {
  if (currentState == WAITING) currentState = SHUFFLE;
  Serial_Printf("currentState: %d\r\n", currentState);
}

void TAO888_SlotMachine_IncrementState() {
  // currentState += 1;
  // if (currentState >= (sizeof(stateConfig) / sizeof(stateConfig[0]))) {
  //   currentState = WAITING;
  // }
}