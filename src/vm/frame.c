#include "frame.h"
#include "../threads/malloc.h"


void* frame_append(void* page, int tid) {
    struct frame* new_frame = calloc(1, sizeof(struct frame));
    new_frame->page = page;
    new_frame->tid = tid;
    list_push_back(&frame_table, &new_frame->frame_elem);
    return page;
}