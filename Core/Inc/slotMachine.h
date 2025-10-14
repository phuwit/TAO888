#ifndef TAO888_SLOT_MACHINE
#define TAO888_SLOT_MACHINE

#define NS_TIMER htim2
#define MINIMUM_UPDATE_MS 15

#include <stdbool.h>
#include <stdint.h>

#include "ili9341.h"
#include "slotSymbols.h"

typedef enum {
  WAITING,
  SHUFFLE,
  SCROLL5,
  SCROLL4,
  SCROLL3,
  SCROLL2,
  SCROLL1,
  RESULT,
} State;

typedef struct {
  bool advanceOnInput;
  uint32_t advanceMicroSecLow;
  uint32_t randomAdvanceMicroSecMod;
  int8_t scrollAmount;
  uint8_t scrollRowStartIndex;
  uint8_t scrollRowEndIndex;
  bool scrollSnap;
} StateConfig;

// static const StateConfig stateConfig[] = {
//     {true, 0, 0, -12, 0, 5, false},
//     {false, 1500000, 500000, -24, 0, 5, false},
//     {false, 1000000, 500000, -8, 0, 5, true},
//     {false, 1000000, 500000, -8, 1, 5, true},
//     {false, 1000000, 500000, -8, 2, 5, true},
//     {false, 1000000, 500000, -8, 3, 5, true},
//     {false, 1000000, 500000, -8, 4, 5, true},
//     {true, 2000000, 0, 0, 0, 0, true},
// };

static const StateConfig stateConfig[] = {
    {true, 0, 0, -12, 0, 5, false},
    {false, 750000, 0, -24, 0, 5, false},
    {false, 150000, 0, -8, 0, 5, true},
    {false, 100000, 0, -8, 1, 5, true},
    {false, 100000, 0, -8, 2, 5, true},
    {false, 100000, 0, -8, 3, 5, true},
    {false, 100000, 0, -8, 4, 5, true},
    {true, 1500000, 0, 0, 0, 0, false},
};

void TAO888_SlotMachine_Init(ILI9341_HandleTypeDef *lcd);
void TAO888_SlotMachine_Update(ILI9341_HandleTypeDef *lcd);
void TAO888_SlotMachine_StartCycle();
void TAO888_SlotMachine_AdvanceStateGracefully();
void TAO888_SlotMachine_IncrementState();
__weak void TAO888_SlotMachine_RoundEndCallback(SlotSymbol* displayedSymbols);
__weak void TAO888_SlotMachine_PayoutCallback(uint16_t credits);
SlotSymbol TAO888_SlotMachine_GetRandomSymbol();
void TAO888_SlotMachine_GetDisplayedSymbols(SlotSymbol* symbolReciever);
void TAO888_SlotMachine_IncrementCredits(uint8_t amount);
void TAO888_SlotMachine_SendCommandToAux(UART_HandleTypeDef* AuxUart, const uint8_t command);
void TAO888_SlotMachine_PollRotaryEncoderAndStart(TIM_TypeDef* EncoderHandle);

#endif