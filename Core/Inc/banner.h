#ifndef TAO888_BANNER
#define TAO888_BANNER

#include "ili9341.h"
#include "ili9341_fonts.h"
#include <stdint.h>

typedef struct {
  uint16_t posX;
  uint16_t posY;
  char* str;
  ILI9341_FontDef* font;
  uint16_t color;
  uint16_t bgcolor;
  int16_t tracking;
} BannerText;

typedef struct {
  BannerText* bannerTexts;
  uint8_t bannerTextLength;
} Banner;

// void TAO888_Banner_Init(Banner* banner);
void TAO888_Banner_Draw(Banner* banner, ILI9341_HandleTypeDef* lcd);
void TAO888_Banner_UpdateCredits(Banner* banner, const uint32_t credits);
void TAO888_Banner_UpdateWinStatus(Banner* banner, bool win);

#endif