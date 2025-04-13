#include "queue.h"
#include "sched.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

static struct queue_t ready_queue;
static struct queue_t run_queue;
static struct queue_t running_list;
static pthread_mutex_t queue_lock;

#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
static int slot_remaining[MAX_PRIO];
static int curr_prio = -1;
#endif

void init_scheduler(void) {
#ifdef MLQ_SCHED
    for (int i = 0; i < MAX_PRIO; i++) {
        mlq_ready_queue[i].size = 0;
        slot_remaining[i] = MAX_PRIO - i;
    }
    curr_prio = -1;
#endif
    ready_queue.size = 0;
    run_queue.size = 0;
    running_list.size = 0;
    pthread_mutex_init(&queue_lock, NULL);
}

int queue_empty(void) {
#ifdef MLQ_SCHED
    for (int prio = 0; prio < MAX_PRIO; prio++) {
        if (!empty(&mlq_ready_queue[prio])) return -1;
    }
    return 0;
#else
    return (empty(&ready_queue) && empty(&run_queue));
#endif
}

#ifdef MLQ_SCHED
struct pcb_t *get_mlq_proc(void) {
    pthread_mutex_lock(&queue_lock);
    
    struct pcb_t *proc = NULL;
    int start_prio = (curr_prio + 1) % MAX_PRIO;
    
    for (int i = 0; i < MAX_PRIO; i++) {
        int prio = (start_prio + i) % MAX_PRIO;
        if (slot_remaining[prio] > 0 && !empty(&mlq_ready_queue[prio])) {
            proc = dequeue(&mlq_ready_queue[prio]);
            slot_remaining[prio]--;
            curr_prio = prio;
            break;
        }
    }
    
    if (proc == NULL) {
        curr_prio = -1;
        for (int i = 0; i < MAX_PRIO; i++) {
            slot_remaining[i] = MAX_PRIO - i;
        }
    }
    
    pthread_mutex_unlock(&queue_lock);
    return proc;
}

void put_mlq_proc(struct pcb_t *proc) {
    if (proc == NULL || proc->prio >= MAX_PRIO) return;
    pthread_mutex_lock(&queue_lock);
    enqueue(&mlq_ready_queue[proc->prio], proc);
    pthread_mutex_unlock(&queue_lock);
}

void add_mlq_proc(struct pcb_t *proc) {
    if (proc == NULL || proc->prio >= MAX_PRIO) return;
    pthread_mutex_lock(&queue_lock);
    enqueue(&mlq_ready_queue[proc->prio], proc);
    pthread_mutex_unlock(&queue_lock);
}

struct pcb_t *get_proc(void) {
    return get_mlq_proc();
}

void put_proc(struct pcb_t *proc) {
    if (proc == NULL) return;
    proc->ready_queue = &ready_queue;
    proc->mlq_ready_queue = mlq_ready_queue;
    proc->running_list = &running_list;
    
    pthread_mutex_lock(&queue_lock);
    if (running_list.size < MAX_QUEUE_SIZE) {
        enqueue(&running_list, proc);
    }
    pthread_mutex_unlock(&queue_lock);
    
    put_mlq_proc(proc);
}

void add_proc(struct pcb_t *proc) {
    if (proc == NULL) return;
    proc->ready_queue = &ready_queue;
    proc->mlq_ready_queue = mlq_ready_queue;
    proc->running_list = &running_list;
    
    pthread_mutex_lock(&queue_lock);
    if (running_list.size < MAX_QUEUE_SIZE) {
        enqueue(&running_list, proc);
    }
    pthread_mutex_unlock(&queue_lock);
    
    add_mlq_proc(proc);
}
#else
struct pcb_t *get_proc(void) {
    pthread_mutex_lock(&queue_lock);
    struct pcb_t *proc = NULL;
    if (!empty(&ready_queue)) {
        proc = dequeue(&ready_queue);
    } else if (!empty(&run_queue)) {
        proc = dequeue(&run_queue);
    }
    pthread_mutex_unlock(&queue_lock);
    return proc;
}

void put_proc(struct pcb_t *proc) {
    if (proc == NULL) return;
    proc->ready_queue = &ready_queue;
    proc->running_list = &running_list;

    pthread_mutex_lock(&queue_lock);
    if (running_list.size < MAX_QUEUE_SIZE) {
        enqueue(&running_list, proc);
    }
    enqueue(&run_queue, proc);
    pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t *proc) {
    if (proc == NULL) return;
    proc->ready_queue = &ready_queue;
    proc->running_list = &running_list;

    pthread_mutex_lock(&queue_lock);
    if (running_list.size < MAX_QUEUE_SIZE) {
        enqueue(&running_list, proc);
    }
    enqueue(&ready_queue, proc);
    pthread_mutex_unlock(&queue_lock);
}
#endif