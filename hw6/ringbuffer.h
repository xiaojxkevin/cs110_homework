#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct ringbuffer_t ringbuffer_t;

#include "threadpool.h"

struct ringbuffer_t {
    size_t size;
    size_t head;
    size_t tail;
    size_t count;
    threadpool_task_t *buffer;
};

/*Create a ringbuffer with size size and return the pointer of ringbuffer created.
Return NULL anything failed in the process.*/
ringbuffer_t *ringbuffer_create(size_t size);

/*Destroy the ringbuffer ringbuffer. Release all the memory allocated.*/
void ringbuffer_destroy(ringbuffer_t *ringbuffer);

/*Return true if the ringbuffer ringbuffer is empty, otherwise return false.*/
bool ringbuffer_is_empty(ringbuffer_t *ringbuffer);

/*Return true if the ringbuffer ringbuffer is full, otherwise return false.*/
bool ringbuffer_is_full(ringbuffer_t *ringbuffer);

/*Push the value value into the ringbuffer ringbuffer.
Return true if the push is successful, otherwise return false.*/
bool ringbuffer_push(ringbuffer_t *ringbuffer, threadpool_task_t value);

/*Retrieve the value from the ringbuffer ringbuffer and store it in value.
Return true if the operation is successful, otherwise return false.*/
bool ringbuffer_pop(ringbuffer_t *ringbuffer, threadpool_task_t *value);

#endif
