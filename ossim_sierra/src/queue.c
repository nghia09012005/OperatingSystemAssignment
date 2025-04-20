#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t *q) {
    return (q == NULL || q->size == 0);
}

void enqueue(struct queue_t *q, struct pcb_t *proc) {
    if (q == NULL)
		return;
	if (q->size == MAX_QUEUE_SIZE)
		return;
	q->proc[q->size] = proc;
	q->size++;
}

struct pcb_t *dequeue(struct queue_t *q) {
    if (q == NULL)
		return NULL;
	if (q->size == 0)
		return NULL;

	
	int min_priority_index = 0;
	for (int i = 1; i < q->size; i++) {
		if (q->proc[i]->priority < q->proc[min_priority_index]->priority) {
			min_priority_index = i;
		}
	}
	struct pcb_t *proc = q->proc[min_priority_index];
	for (int i = min_priority_index; i < q->size - 1; i++) {
		q->proc[i] = q->proc[i + 1];
	}
	q->size--;
	q->proc[q->size] = NULL;
	return proc;
}