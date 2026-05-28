#include "Logic.h"

// 初始化队列
void queue_init(bulk_queue *q)
{
    q->front = 0;
    q->rear = 0;
    q->size = 0;
}
// 判断队列空
int is_empty(bulk_queue *q)
{
    return q->size == 0;
}
// 判断队列满
int is_full(bulk_queue *q)
{
    return q->size == MAX_QUEUE_SIZE;
}
// 入队
int en_queue(bulk_queue *q, int value)
{
    if (is_full(q))
    {
        return -1; // 队列已满，入队失败
    }
    q->data[q->rear] = value;
    q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
    q->size++;
    return 0; // 入队成功
}
// 出队
int de_queue(bulk_queue *q, int *value)
{
    if (is_empty(q))
    {
        return -1; // 队列为空，出队失败
    }
    *value = q->data[q->front];
    q->front = (q->front + 1) % MAX_QUEUE_SIZE;
    q->size--;
    return 0; // 出队成功
}