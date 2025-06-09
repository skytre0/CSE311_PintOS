#ifndef FRAME_H
#define FRAME_H

#include <list.h>
#include "page.h"
#include "swap.h"

static struct lock frame_lock; // 세마로할까?

struct frame {
    void *page;           /* Saved page directory address. */
    struct supplemental_page* sp;
    struct thread* thread;      // need to know thread tid
    struct list_elem frame_elem;   /* List element for the frame list. */
};

//  contains a pointer to the page,

struct list frame_table;



void frame_init(void);

void* file_frame_alloc(struct thread* tc, struct supplemental_page* sp);
void* stack_frame_alloc(struct thread* tc, struct supplemental_page* sp);

void* free_frame(struct thread* tc, void *page);

struct frame* frame_evict(void);

void* frame_append(struct thread* tc, void* page);

#endif