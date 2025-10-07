#include "frameBuffer.h"

FrameBuffer TAO888_FrameBuffer_Initialize(const uint16_t x, const uint16_t y,
                                            const uint16_t writableWidth,
                                            const uint16_t writableHeight,
                                            const uint16_t paddingTop,
                                            const uint16_t paddingBottom,
                                            const uint16_t paddingLeft,
                                            const uint16_t paddingRight) {
  FrameBuffer frameBuffer;
  frameBuffer.x = x;
  frameBuffer.y = y;
  frameBuffer.writableWidth = writableWidth;
  frameBuffer.writableHeight = writableHeight;
  frameBuffer.paddingTop = paddingTop;
  frameBuffer.paddingBottom = paddingBottom;
  frameBuffer.paddingLeft = paddingLeft;
  frameBuffer.paddingRight = paddingRight;
  frameBuffer.readRow = 0;
  frameBuffer.readColumn = 0;
  frameBuffer.bufferWidth = writableWidth + paddingLeft + paddingRight;
  frameBuffer.bufferHeight = writableHeight + paddingTop + paddingBottom;
  frameBuffer.buffer =
      malloc(sizeof(uint16_t) * (frameBuffer.bufferWidth) *
             (frameBuffer.bufferHeight));
  return frameBuffer;
}

bool TAO888_FrameBuffer_IncrementReadRow(FrameBuffer *frameBuffer,
                                         const int16_t amount) {
    // Save the old row to detect a loop
    const int16_t oldReadRow = frameBuffer->readRow;

    // Apply the increment
    frameBuffer->readRow += amount;

    // Handle wrapping around the bufferHeight
    if (frameBuffer->readRow >= frameBuffer->bufferHeight) {
        frameBuffer->readRow %= frameBuffer->bufferHeight;
    } else if (frameBuffer->readRow < 0) {
        // Ensure the result is positive after looping backwards
        frameBuffer->readRow = (frameBuffer->readRow % frameBuffer->bufferHeight) + frameBuffer->bufferHeight;
    }

    // A loop occurred if the new row is less than the old one after a positive increment
    // or greater after a negative increment.
    if ((amount > 0 && frameBuffer->readRow < oldReadRow) ||
        (amount < 0 && frameBuffer->readRow > oldReadRow)) {
        return true;
    }

    return false;
}

// A simplified and more robust increment for the column
bool TAO888_FrameBuffer_IncrementReadColumn(FrameBuffer *frameBuffer,
                                            const int16_t amount) {
    const int16_t oldReadColumn = frameBuffer->readColumn;
    frameBuffer->readColumn += amount;

    if (frameBuffer->readColumn >= frameBuffer->bufferWidth) {
        frameBuffer->readColumn %= frameBuffer->bufferWidth;
    } else if (frameBuffer->readColumn < 0) {
        frameBuffer->readColumn = (frameBuffer->readColumn % frameBuffer->bufferWidth) + frameBuffer->bufferWidth;
    }

    if ((amount > 0 && frameBuffer->readColumn < oldReadColumn) ||
        (amount < 0 && frameBuffer->readColumn > oldReadColumn)) {
        return true;
    }
    return false;
}

void TAO888_FrameBuffer_Commit(const FrameBuffer *frameBuffer,
                               ILI9341_HandleTypeDef *ili9341) {

    // Calculate the number of rows from the readRow to the bottom of the buffer
    uint16_t rowsToEnd = frameBuffer->bufferHeight - frameBuffer->readRow;

    if (rowsToEnd >= frameBuffer->writableHeight) {
        // No wrapping needed, the entire writable area is contiguous in memory
        const uint16_t *startPtr = frameBuffer->buffer + (frameBuffer->readRow * frameBuffer->bufferWidth) + frameBuffer->readColumn;
        ILI9341_DrawImage(ili9341, frameBuffer->x, frameBuffer->y,
                          frameBuffer->writableWidth, frameBuffer->writableHeight,
                          startPtr);
    } else {
        // Wrapping is needed, draw in two parts

        // Part 1: From the current readRow to the end of the buffer
        const uint16_t *firstPartPtr = frameBuffer->buffer + (frameBuffer->readRow * frameBuffer->bufferWidth) + frameBuffer->readColumn;
        ILI9341_DrawImage(ili9341, frameBuffer->x, frameBuffer->y,
                          frameBuffer->writableWidth, rowsToEnd,
                          firstPartPtr);

        // Part 2: From the beginning of the buffer to fill the remaining writableHeight
        uint16_t remainingRows = frameBuffer->writableHeight - rowsToEnd;
        const uint16_t *secondPartPtr = frameBuffer->buffer + frameBuffer->readColumn; // Start from the top of the buffer
        ILI9341_DrawImage(ili9341, frameBuffer->x, frameBuffer->y + rowsToEnd,
                          frameBuffer->writableWidth, remainingRows,
                          secondPartPtr);
    }
}

void TAO888_FrameBuffer_Fill(FrameBuffer *frameBuffer, const uint16_t color) {
  for (int row = 0; row < frameBuffer->bufferHeight; row += 1) {
    for (int column = 0; column < frameBuffer->bufferWidth; column += 1) {
      frameBuffer->buffer[frameBuffer->bufferWidth * row + column] = color;
    }
  }
}

void TAO888_FrameBuffer_DrawImage(FrameBuffer *frameBuffer, const uint16_t x,
                                  const uint16_t y, const uint16_t width,
                                  const uint16_t height,
                                  const uint16_t *image) {
  for (uint16_t row = 0; row < height; row++) {
    for (uint16_t col = 0; col < width; col++) {
      uint16_t fb_x = x + col;
      uint16_t fb_y = y + row;
      // Check bounds
      if (fb_x < frameBuffer->bufferWidth &&
          fb_y < frameBuffer->bufferHeight) {
        frameBuffer->buffer[fb_y * frameBuffer->writableWidth + fb_x] =
            image[row * width + col];
      }
    }
  }
}

void TAO888_FrameBuffer_DrawLine(FrameBuffer *frameBuffer, uint16_t x0,
                                 uint16_t y0, uint16_t x1, uint16_t y1,
                                 uint16_t color) {
  int dx = abs(x1 - x0);
  int dy = -abs(y1 - y0);
  int sx = x0 < x1 ? 1 : -1;
  int sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;
  while (1) {
    if (x0 < frameBuffer->writableWidth && y0 < frameBuffer->bufferHeight) {
      frameBuffer->buffer[y0 * frameBuffer->bufferWidth + x0] = color;
    }
    if (x0 == x1 && y0 == y1)
      break;
    int e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      x0 += sx;
    }
    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    }
  }
}
