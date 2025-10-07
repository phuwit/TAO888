#ifndef SERIAL_UTILS
#define SERIAL_UTILS

#define UART_RETRY 50
#define UART_HANDLE huart3

#include <stdint.h>

uint8_t Serial_Recv(uint32_t timeout);
void Serial_Print(const char *str);
void Serial_Printf(const char *format, ...);
void Serial_Println(const char *str);
void Serial_Debug_Print(const char *str);
void Serial_Debug_Printf(const char *format, ...);
void Serial_Debug_Println(const char *str);

#endif