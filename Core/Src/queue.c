#include "queue.h"

// Function to initialize the queue with a given size and item size
void initializeQueue(SymbolQueue* q, uint32_t size, size_t item_size)
{
    q->items = malloc(item_size * size);
    q->front = -1;
    q->rear = 0;
    q->max_size = size;
    q->item_size = item_size;
}

// Function to free the queue memory
void freeQueue(SymbolQueue* q)
{
    free(q->items);
    q->items = NULL;
    q->front = -1;
    q->rear = 0;
    q->max_size = 0;
    q->item_size = 0;
}

// Function to check if the queue is empty
bool isEmpty(SymbolQueue* q) { return (q->front == q->rear - 1); }

// Function to check if the queue is full
bool isFull(SymbolQueue* q) { return (q->rear == q->max_size); }

// Function to add an element to the queue (Enqueue operation)
void enqueue(SymbolQueue* q, void* value)
{
    if (isFull(q)) {
        printf("Queue is full\n");
        return;
    }
    void* dest = (char*)q->items + (q->rear * q->item_size);
    memcpy(dest, value, q->item_size);
    q->rear++;
}

// Function to remove an element from the queue (Dequeue operation)
void dequeue(SymbolQueue* q)
{
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return;
    }
    q->front++;
}

// Function to get the element at the front of the queue (Peek operation)
void* peek(SymbolQueue* q)
{
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return NULL;
    }
    return (char*)q->items + ((q->front + 1) * q->item_size);
}

void* peekAt(SymbolQueue* q, int32_t index)
{
    int32_t actualIndex = q->front + 1 + index;
    if (actualIndex < q->front + 1 || actualIndex >= q->rear) {
        printf("peekAt: Index out of bounds\n");
        return NULL;
    }
    return (char*)q->items + (actualIndex * q->item_size);
}

// Function to print the current queue (requires a print function for the item type)
void printQueue(SymbolQueue* q, void (*printItem)(void*))
{
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return;
    }

    printf("Current Queue: ");
    for (int32_t i = q->front + 1; i < q->rear; i++) {
        void* item = (char*)q->items + (i * q->item_size);
        printItem(item);
    }
    printf("\n");
}

// Function to get the queue as a sorted array (copies items to outArray)
int32_t getQueueSortedArray(SymbolQueue* q, void* outArray) {
    if (isEmpty(q)) {
        return 0;
    }
    int32_t count = 0;
    for (int32_t i = q->front + 1; i < q->rear; i++) {
        void* src = (char*)q->items + (i * q->item_size);
        void* dest = (char*)outArray + (count * q->item_size);
        memcpy(dest, src, q->item_size);
        count++;
    }
    return count;
}