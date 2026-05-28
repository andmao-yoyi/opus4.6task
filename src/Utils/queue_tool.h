#ifndef __QUEUETOOL_H
#define __QUEUETOOL_H

#include "logic.h"

#define MAX_QUEUE_SIZE 10  // 队列的最大容量
typedef struct {
    int data[MAX_QUEUE_SIZE];    // 存储队列元素的数组
    int front;                   // 队头指针
    int rear;                    // 队尾指针
    int size;                    // 当前队列大小
} bulk_queue;

// 初始化队列
void queue_init(bulk_queue *q);
// 判断队列空
int is_empty(bulk_queue *q);
// 判断队列满
int is_full(bulk_queue *q);
// 入队
int en_queue(bulk_queue *q, int value);
// 出队
int de_queue(bulk_queue *q, int *value);

#endif
