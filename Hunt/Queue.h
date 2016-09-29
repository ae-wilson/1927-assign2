#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include "Places.h"

typedef struct QueueRep *Queue;

Queue newQueue();
void disposeQueue(Queue);
void enterQueue(Queue, LocationID);
LocationID leaveQueue(Queue);
int emptyQueue(Queue);

#endif

