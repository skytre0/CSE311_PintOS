#ifndef FRAME_H
#define FRAME_H

#include <list.h>

struct frame {
    void *address;
    void *page;           /* Saved page directory address. */
    struct hash* spte;
    struct thread* thread;      // need to know thread tid
    struct list_elem frame_elem;   /* List element for the frame list. */
};

//  contains a pointer to the page,

struct list frame_table;

void* frame_alloc(struct thread* tc);

void* free_frame(struct thread* tc);

// void* frame_evict();

#endif