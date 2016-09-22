#include <stdio.h>
#include <stdlib.h>
#include "Places.h"

#ifndef QUEUE_H
#define QUEUE_H

typedef struct QueueRep *Queue;

Queue newQueue();
void disposeQueue(Queue);
void enterQueue(Queue, LocationID);
LocationID leaveQueue(Queue);
int emptyQueue(Queue);

#endif

