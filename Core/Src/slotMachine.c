#include "slotMachine.h"
#include "banner.h"
#include "main.h"
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

int8_t scrollAmount = -8;

void TAO888_SlotMachine_Init(ILI9341_HandleTypeDef* lcd) {
  TAO888_Banner_Draw(&banner, lcd);

  // vertical lines
  for (int xOffset = 1; xOffset <= 5; xOffset++) {
    ILI9341_DrawLine(lcd, (xOffset * SLOT_CELL_SIZE) - 1, HEADER_SIZE,
                     (xOffset * SLOT_CELL_SIZE) - 1, LCD_HEIGHT,
                     ILI9341_COLOR_BLACK);
  }

  TAO888_SlotCols_Init(slotCols, lcd);
}

void TAO888_SlotMachine_Update(ILI9341_HandleTypeDef* lcd) {
  TAO888_SlotCols_Scroll(slotCols, lcd, scrollAmount);
}