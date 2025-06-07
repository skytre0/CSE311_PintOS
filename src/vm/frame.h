#ifndef FRAME_H
#define FRAME_H

#include <list.h>

struct frame {
    void *address;
    void *page;           /* Saved page directory. */
    int tid;
    struct list_elem frame_elem;   /* List element for the frame list. */
};

//  contains a pointer to the page,

struct list frame_table;

void* frame_require(int tid);

#endif FRAME_H