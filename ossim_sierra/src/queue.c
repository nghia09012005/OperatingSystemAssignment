#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t *q) {
    if (q == NULL) return 1;
    return (q->size == 0);
}

void enqueue(struct queue_t *q, struct pcb_t *proc) {
    if (q == NULL || proc == NULL) return;
    if (q->size >= MAX_QUEUE_SIZE) {
        
        return;
    }
    q->proc[q->size] = proc;
    q->size++;
}

struct pcb_t *dequeue(struct queue_t *q) {
    if (empty(q)) return NULL;
    
    struct pcb_t *proc = q->proc[0];
    if (proc == NULL) return NULL;
    
    // Shift all elements left by one
    for (int i = 0; i < q->size - 1; i++) {
        q->proc[i] = q->proc[i + 1];
    }
    
    // Clear the last position and decrement size
    q->proc[--q->size] = NULL;
    return proc;
}