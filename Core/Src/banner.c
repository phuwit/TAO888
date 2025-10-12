#include "banner.h"
#include "ili9341.h"
#include "main.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>


char bannerCreditsBuffer[24] = "\0";

void TAO888_Banner_Draw(Banner* banner, ILI9341_HandleTypeDef* lcd) {
  ILI9341_FillRectangle(lcd, 0, 0, LCD_WIDTH, BANNER_SIZE, ILI9341_COLOR_WHITE);

  for (int i = 0; i < banner->bannerTextLength; i += 1) {
    ILI9341_WriteString(lcd, banner->bannerTexts[i].posX, banner->bannerTexts[i].posY, banner->bannerTexts[i].str,
                        *banner->bannerTexts[i].font, banner->bannerTexts[i].color,
                        banner->bannerTexts[i].bgcolor, banner->bannerTexts[i].tracking);
  }
  // under banner
  ILI9341_DrawLine(lcd, 0, BANNER_SIZE - 1,
                     LCD_WIDTH, BANNER_SIZE - 1,
                     ILI9341_COLOR_BLACK);
}

void TAO888_Banner_UpdateCredits(Banner* banner, const uint32_t credits) {
  static const uint8_t amountIndex = 2;
  const char creditsTextLength = (credits == 0 ? 1 : (int)(log10(credits)+1));

  sprintf(bannerCreditsBuffer, BANNER_CREDITS_PATTERN, credits);
  banner->bannerTexts[amountIndex].str = bannerCreditsBuffer;
  banner->bannerTexts[amountIndex].posX = LCD_WIDTH - 12 - (creditsTextLength * 10) - ((strlen(BANNER_CREDITS_PATTERN) - 2) * 10);
}

void TAO888_Banner_UpdateWinStatus(Banner* banner, bool win) {
  static const uint8_t winStatusIndex = 1;
  if (win) {
    strcpy(banner->bannerTexts[winStatusIndex].str, BANNER_WIN_TEXT);
    banner->bannerTexts[winStatusIndex].posX = LCD_WIDTH - 12 - ((strlen(BANNER_WIN_TEXT) - 1) * 10);
  } else {
    strcpy(banner->bannerTexts[winStatusIndex].str, BANNER_LOSE_TEXT);
    banner->bannerTexts[winStatusIndex].posX = LCD_WIDTH - 12 - ((strlen(BANNER_LOSE_TEXT) - 1) * 10);
  }
}