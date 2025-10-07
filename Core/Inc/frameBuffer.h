#ifndef TAO888_FRAMEBUFFER
#define TAO888_FRAMEBUFFER

#include <stdint.h>
#include "ili9341.h"

typedef struct {
  uint16_t x;
  uint16_t y;
  uint16_t writableWidth;
  uint16_t writableHeight;
  uint16_t paddingTop;
  uint16_t paddingBottom;
  uint16_t paddingLeft;
  uint16_t paddingRight;
  int16_t readRow;
  int16_t readColumn;
  uint16_t bufferWidth;
  uint16_t bufferHeight;
  uint16_t *buffer;
} FrameBuffer;

FrameBuffer TAO888_FrameBuffer_Initialize(
  const uint16_t x, const uint16_t y, const uint16_t writableWidth,
  const uint16_t writableHeight, const uint16_t paddingTop, const uint16_t paddingBottom,
  const uint16_t paddingLeft, const uint16_t paddingRight);

// returns false if incremented, true if incremented and looped
bool TAO888_FrameBuffer_IncrementReadRow(FrameBuffer *frameBuffer,
                                         const int16_t amount
                                         );

bool TAO888_FrameBuffer_IncrementReadColumn(FrameBuffer *frameBuffer,
                                            const int16_t amount
                                            );

void TAO888_FrameBuffer_Commit(const FrameBuffer *frameBuffer,
                               ILI9341_HandleTypeDef *ili9341);

void TAO888_FrameBuffer_Fill(FrameBuffer *frameBuffer, const uint16_t color);

void TAO888_FrameBuffer_DrawImage(FrameBuffer *frameBuffer, const uint16_t x,
                                  const uint16_t y, const uint16_t width,
                                  const uint16_t height, const uint16_t *image);

void TAO888_FrameBuffer_DrawLine(FrameBuffer *frameBuffer, uint16_t x0,
                                 uint16_t y0, uint16_t x1, uint16_t y1,
                                 uint16_t color);

#endif