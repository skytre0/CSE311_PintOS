#include "frame.h"
#include "../threads/malloc.h"
#include "../threads/palloc.h"
#include "../threads/thread.h"
#include "../userprog/exception.h"


void* file_frame_alloc(struct thread* tc) {
    struct frame* new_frame = calloc(1, sizeof(struct frame));
    new_frame->page = palloc_get_page (PAL_USER);
    // swapping 이후 추가 필요.
    list_push_back(&frame_table, &new_frame->frame_elem);
    new_frame->thread = tc;
    new_frame->spte = &tc->spt;

    return new_frame->page;
}


void* stack_frame_alloc(struct thread* tc) {
    struct frame* new_frame = calloc(1, sizeof(struct frame));
    new_frame->page = palloc_get_page (PAL_USER | PAL_ZERO);
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

struct frame* frame_evict(){ // 죽일놈 선택하기만
    while(1){
        struct list_elem* frame_elem = list_begin(&frame_table); // 우리는 맨 앞만 본다, 앞에꺼 뺴서 맨 뒤에 넣기
        struct frame* frame = list_entry(frame_elem, struct frame, frame_elem);

        list_remove(frame_elem);
        list_push_back(&frame_table, frame_elem);

        if (frame->thread == NULL) {
            // 사용하지 않는 프레임 발견
            return frame;
        }

        if (pagedir_is_accessed(frame->thread->pagedir, frame->spte->vaddr)) { // 최근에 접근했다면, Accessed Bit를 0으로
            pagedir_set_accessed(frame->thread->pagedir, frame->spte->vaddr, false);
            continue;
        }

        // 맨 앞을 뺴고, 맨뒤에 넣음.
        return frame;
    }
    return NULL;
}