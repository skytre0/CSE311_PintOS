#include "frame.h"
#include "../threads/malloc.h"
#include "../threads/palloc.h"
#include "../threads/thread.h"
#include "../userprog/exception.h"


void* frame_alloc(struct thread* tc) {
    struct frame* new_frame = calloc(1, sizeof(struct frame));
    new_frame->page = palloc_get_page (PAL_USER);
    // swapping 이후 추가 필요.
    list_push_back(&frame_table, &new_frame->frame_elem);
    new_frame->thread = tc;
    new_frame->spte = &tc->spt;

    return new_frame->page;
}

void* free_frame(struct thread* tc, void *page) {
    struct list_elem* ft_elem = list_begin(&frame_table);
    for ( ;ft_elem != list_end(&frame_table); ) {
        struct frame* frame = list_entry(ft_elem, struct frame, frame_elem);
        if (frame->thread == tc) {
            palloc_free_page(page);
            free(frame);
            ft_elem = list_remove(ft_elem);
        }
        else ft_elem = list_next(ft_elem);
    }
}

void* frame_append(struct thread* tc, void* page){
    struct frame* new_frame = calloc(1, sizeof(struct frame));
    new_frame->page = page;
    new_frame->thread = tc;
    list_push_back(&frame_table, &new_frame->frame_elem);
    return new_frame->page;
}