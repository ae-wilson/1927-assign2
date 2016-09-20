#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "Queue.h"
#include "Places.h"

#define TRUE 1
#define FALSE 0

typedef struct QueueNode *Node;
struct QueueNode {
    LocationID v;
    Node next;
} QueueNode;

typedef struct QueueRep {
	Node head;
	Node tail;
} QueueRep;


Queue newQueue() {
    Queue q = malloc(sizeof(QueueRep));
    assert(q != NULL);
    
    q->head = NULL;
    q->tail = NULL;

    return q;
}

void disposeQueue(Queue q) {
    if(q == NULL) return;

    Node curr, prev = q->head;

    while(curr != NULL) {
        prev = curr;
        curr = curr->next;
        free(prev);
    }

    free(q);
}

void enterQueue(Queue q, LocationID location) {
    assert(q != NULL);
    
    Node newNode = malloc(sizeof(QueueNode));
    assert(newNode != NULL);

    newNode->v = location;
    newNode->next = NULL;

    if(q->head == NULL) {
        q->head = newNode;
        q->tail = newNode;
    } else {
        q->tail->next = newNode;
        q->tail = newNode;
    }

}

LocationID leaveQueue(Queue q) {
    assert(q != NULL);
    assert(q->head != NULL);

    LocationID location =  q->head->v;

    if(q->head == q->tail) {
        free(q->head);
        q->head = q->tail = NULL;
    } else {
        Node nextNode = q->head->next;
        free(q->head);
        q->head = nextNode;
    }

    return location;
}

int emptyQueue(Queue q) {
    assert(q != NULL);
    
    if(q->head == NULL) {
        return TRUE;
    } else {
        return FALSE;
    }
}

