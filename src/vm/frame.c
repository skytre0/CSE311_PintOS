#include "frame.h"
#include "../threads/malloc.h"
#include "../threads/palloc.h"
#include "../userprog/exception.h"


void* frame_require(int tid) {
    struct frame* new_frame = calloc(1, sizeof(struct frame));
    new_frame->page = palloc_get_page (PAL_USER);
    new_frame->tid = tid;
    list_push_back(&frame_table, &new_frame->frame_elem);
    return new_frame->page;
}