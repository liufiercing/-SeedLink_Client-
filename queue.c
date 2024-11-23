#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "seedlink.h"

DataQueue* queue_create(void) {
    DataQueue* queue = (DataQueue*)malloc(sizeof(DataQueue));
    if (!queue) return NULL;
    
    queue->front = queue->rear = NULL;
    queue->size = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    
    return queue;
}

void queue_destroy(DataQueue* queue) {
    if (!queue) return;
    
    pthread_mutex_lock(&queue->mutex);
    while (queue->front) {
        QueueNode* temp = queue->front;
        queue->front = queue->front->next;
        free(temp);
    }
    pthread_mutex_unlock(&queue->mutex);
    
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->not_empty);
    free(queue);
}

int queue_push(DataQueue* queue, const char* network, const char* station,
              const char* location, const char* channel, const unsigned char* data) {
    QueueNode* node = (QueueNode*)malloc(sizeof(QueueNode));
    if (!node) return -1;
    
    strncpy(node->network, network, 2);
    strncpy(node->station, station, 5);
    strncpy(node->location, location, 2);
    strncpy(node->channel, channel, 3);
    memcpy(node->data, data, 512);
    node->next = NULL;
    
    pthread_mutex_lock(&queue->mutex);
    if (!queue->rear) {
        queue->front = queue->rear = node;
    } else {
        queue->rear->next = node;
        queue->rear = node;
    }
    queue->size++;
    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->mutex);
    
    return 0;
}

int queue_pop(DataQueue* queue, char* network, char* station,
             char* location, char* channel, unsigned char* data) {
    pthread_mutex_lock(&queue->mutex);
    while (queue->size == 0) {
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }
    
    QueueNode* node = queue->front;
    strcpy(network, node->network);
    strcpy(station, node->station);
    strcpy(location, node->location);
    strcpy(channel, node->channel);
    memcpy(data, node->data, 512);
    
    queue->front = node->next;
    if (!queue->front) {
        queue->rear = NULL;
    }
    queue->size--;
    
    free(node);
    pthread_mutex_unlock(&queue->mutex);
    
    return 0;
}

int queue_size(DataQueue* queue) {
    pthread_mutex_lock(&queue->mutex);
    int size = queue->size;
    pthread_mutex_unlock(&queue->mutex);
    return size;
} 