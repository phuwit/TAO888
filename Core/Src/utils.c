#include "main.h"
#include "slotSymbols.h"
#include <stdint.h>

SlotSymbol TAO888_Utils_GetRandomSymbol() {
  uint8_t index = HAL_RNG_GetRandomNumber(&hrng) % slotSymbolsLength;
  return slotSymbols[index];
}