#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include "miniseed.h"

// 队列节点结构
typedef struct QueueNode {
    char network[3];
    char station[6];
    char location[3];
    char channel[4];
    unsigned char data[512];  // miniSEED数据
    struct QueueNode* next;
} QueueNode;

// 队列结构
typedef struct {
    QueueNode* front;
    QueueNode* rear;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    int size;
} DataQueue;

// 函数声明
DataQueue* queue_create(void);
void queue_destroy(DataQueue* queue);
int queue_push(DataQueue* queue, const char* network, const char* station,
              const char* location, const char* channel, const unsigned char* data);
int queue_pop(DataQueue* queue, char* network, char* station,
             char* location, char* channel, unsigned char* data);
int queue_size(DataQueue* queue);

#endif 