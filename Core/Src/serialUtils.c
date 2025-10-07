#include "serialUtils.h"
#include "main.h"
#include "stm32f7xx_hal_uart.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef UART_HANDLE;

uint8_t Serial_Recv(uint32_t timeout) {
  HAL_StatusTypeDef status = HAL_ERROR;
  uint8_t rx = 0;

  while (status != HAL_OK && status != HAL_TIMEOUT) { status = HAL_UART_Receive(&UART_HANDLE, &rx, 1, timeout); }

  return rx;
}

void Serial_Print(const char *str) {
  uint8_t retry = 0;
  while (HAL_UART_Transmit(&UART_HANDLE, (uint8_t *)str, strlen(str), HAL_MAX_DELAY) != HAL_OK && retry < UART_RETRY) {
    retry++;
  }
}

void Serial_Printf(const char *format, ...) {
  char buffer[2048];
  va_list args;

  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  Serial_Print(buffer);
}

void Serial_Println(const char *str) {
  Serial_Print(str);
  Serial_Print("\r\n");
}

void Serial_Debug_Print(const char *str) {
  #ifdef DEBUG_PRINT
  uint8_t retry = 0;
  while (HAL_UART_Transmit(&UART_HANDLE, (uint8_t *)str, strlen(str), HAL_MAX_DELAY) != HAL_OK && retry < UART_RETRY) {
    retry++;
  }
  #endif
}

void Serial_Debug_Printf(const char *format, ...) {
  #ifdef DEBUG_PRINT
  char buffer[2048];
  va_list args;

  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  Serial_Print(buffer);
  #endif
}

void Serial_Debug_Println(const char *str) {
  #ifdef DEBUG_PRINT
  Serial_Print(str);
  Serial_Print("\r\n");
  #endif
}