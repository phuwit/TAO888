#include "banner.h"
#include "ili9341.h"
#include "main.h"

void TAO888_Banner_Draw(Banner* banner, ILI9341_HandleTypeDef* lcd) {
  ILI9341_FillRectangle(lcd, 0, 0, LCD_WIDTH, HEADER_SIZE, ILI9341_COLOR_WHITE);

  for (int i = 0; i < banner->bannerTextLength; i += 1) {
    ILI9341_WriteString(lcd, banner->bannerTexts[i].posX, banner->bannerTexts[i].posY, banner->bannerTexts[i].str,
                        *banner->bannerTexts[i].font, banner->bannerTexts[i].color,
                        banner->bannerTexts[i].bgcolor, banner->bannerTexts[i].tracking);
  }
  // under banner
  ILI9341_DrawLine(lcd, 0, HEADER_SIZE - 1,
                     LCD_WIDTH, HEADER_SIZE - 1,
                     ILI9341_COLOR_BLACK);
}