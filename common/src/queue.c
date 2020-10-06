#include <stdlib.h>
#include <string.h>
#include "queue.h"



void cb_init(circular_buffer *cb, size_t capacity, size_t sz)
{
  cb->buffer = malloc(capacity * sz);
  if (cb->buffer == NULL)
    exit(1);
  cb->buffer_end = (char *)cb->buffer + capacity * sz;
  cb->capacity = capacity;
  cb->count = 0;
  cb->sz = sz;
  cb->head = cb->buffer;
  cb->tail = cb->buffer;

  pthread_mutex_init(&cb->lock, NULL);
}

void cb_free(circular_buffer *cb)
{
  free(cb->buffer);
  // clear out other fields too, just to be safe
}

int cb_push_back(circular_buffer *cb, const void *item)
{
  // locking
  pthread_mutex_lock(&cb->lock);
  if (cb->count == cb->capacity) {
    // unlocking
    pthread_mutex_unlock(&cb->lock);
    return 1;
  }

  memcpy(cb->head, item, cb->sz);
  cb->head = (char*)cb->head + cb->sz;
  if (cb->head == cb->buffer_end)
    cb->head = cb->buffer;
  cb->count++;

  // unlocking
  pthread_mutex_unlock(&cb->lock);

  return 0;

}

int cb_pop_front(circular_buffer *cb, void *item)
{
  // locking
  pthread_mutex_lock(&cb->lock);
  if (cb->count == 0) {
    // unlocking
    pthread_mutex_unlock(&cb->lock);
    return 1;
  }

  memcpy(item, cb->tail, cb->sz);

  cb->tail = (char*)cb->tail + cb->sz;
  if (cb->tail == cb->buffer_end)
    cb->tail = cb->buffer;
  cb->count--;

  // unlocking
  pthread_mutex_unlock(&cb->lock);
  return 0;
}
