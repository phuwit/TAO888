#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void* items;
    int32_t front;
    int32_t rear;
    uint32_t max_size;
    size_t item_size;
} SymbolQueue;

// Function prototypes
void initializeQueue(SymbolQueue* q, uint32_t size, size_t item_size);
void freeQueue(SymbolQueue* q);
bool isEmpty(SymbolQueue* q);
bool isFull(SymbolQueue* q);
void enqueue(SymbolQueue* q, void* value);
void dequeue(SymbolQueue* q);
void* peek(SymbolQueue* q);
void* peekAt(SymbolQueue* q, int32_t index);
void printQueue(SymbolQueue* q, void (*printItem)(void*));
int32_t getQueueSortedArray(SymbolQueue* q, void* outArray);

#endif