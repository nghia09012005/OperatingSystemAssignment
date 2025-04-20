/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

 #include "common.h"
 #include "syscall.h"
 #include "stdio.h"
 #include "libmem.h"
 #include "queue.h"
 
 int __sys_killall(struct pcb_t *caller, struct sc_regs *regs)
 {
     char proc_name[100];
     uint32_t data;
 
     // hardcode for demo only
     uint32_t memrg = regs->a1;
 
     /* TODO: Get name of the target proc */
     // proc_name = libread..
     int i = 0;
     data = 0;
     while (data != -1)
     {
         libread(caller, memrg, i, &data);
         proc_name[i] = data;
         if (data == -1)
             proc_name[i] = '\0';
         i++;
     }
     printf("The procname retrieved from memregionid %d is \"%s\"\n", memrg, proc_name);

 
    
    
     for (int prio = 0; prio < MAX_PRIO; prio++)
     {
         struct queue_t *queue = &caller->mlq_ready_queue[prio];
 
         int size = queue->size;
         for (int i = 0; i < size; i++)
         {
             struct pcb_t *proc = queue->proc[i];
             char *pname = strrchr(proc->path, '/');
             if (pname != NULL)
             {
                 pname++;
             }
             else
             {
                 pname = proc->path;
             }
 
             if (strcmp(pname, proc_name) == 0)
             {
                 free(proc);
                 for (int j = i; j < size - 1; j++)
                 {
                     queue->proc[j] = queue->proc[j + 1];
                 }
                 queue->size--;
                 size--; 
                 i--;    
             }
         }
     }

     if (caller->running_list != NULL)
     {
         int size = caller->running_list->size;
         for (int i = 0; i < size; i++)
         {
             struct pcb_t *proc = caller->running_list->proc[i];
             char *pname = strrchr(proc->path, '/');
             if (pname != NULL)
             {
                 pname++;
             }
             else
             {
                 pname = proc->path;
             }
 
             if (strcmp(pname, proc_name) == 0)
             {
                 free(proc);
                 for (int j = i; j < size - 1; j++)
                 {
                     caller->running_list->proc[j] = caller->running_list->proc[j + 1];
                 }
                 caller->running_list->proc[size - 1] = NULL; 
                 caller->running_list->size--;
                 size--;
                 i--; 
             }
         }
     }
 
     return 0;
 }
 
