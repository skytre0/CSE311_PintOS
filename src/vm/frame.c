#include "frame.h"
#include "../threads/malloc.h"
#include "../threads/palloc.h"
#include "../threads/thread.h"
#include "../userprog/exception.h"


void frame_init(void){
    list_init ( &frame_table );
    lock_init (&frame_lock);
}

struct supplemental_page* stack_grow(void* fault_addr, void* esp) {
    // fault_addr >= f->esp - 32); 32 안에 있으면 스택키우는 거임.
    if (PHYS_BASE - fault_addr > (1<<23))  numexit(-1);
    if (fault_addr < esp - 32)  numexit(-1); // 8mb 보고
    uint8_t *upage = pg_round_down(fault_addr);
    struct supplemental_page* sp = create_new_sp(NULL, NULL, upage, 0, PGSIZE, true, -1);
    hash_insert(&thread_current()->spt, &sp->hash_elem);
    return sp;
}

void* file_frame_alloc(struct thread* tc, struct supplemental_page* sp) {
    struct frame* new_frame;             // swapping 여부에 따라 나뉘기에
    void* kaddr = palloc_get_page (PAL_USER);   // 동일 이유
    // swapping 이후 추가 필요.
    lock_acquire(&frame_lock);
    if (kaddr == NULL) {      // swapping으로 기존 frame_table 사용.
        new_frame = swap_out();       // 기존 frame 재활용.
    }
    else {      // 새 frame 만들고, frame_table에 저장.
        new_frame = calloc(1, sizeof(struct frame));
        new_frame->page = kaddr;
        list_push_back(&frame_table, &new_frame->frame_elem);
    }
    swap_in(new_frame, sp);
    new_frame->thread = tc;
    new_frame->sp = sp;

    // synchronize 하기 -> setup stack 변경 싫어서 그냥 page_fault에 넣음
    // pagedir_set_page(tc->pagedir, sp->upage, new_frame->page, sp->writable);

    lock_release(&frame_lock);
    return new_frame->page;
}


void* stack_frame_alloc(struct thread* tc, struct supplemental_page* sp) {
    struct frame* new_frame;             // swapping 여부에 따라 나뉘기에
    void* kaddr = palloc_get_page (PAL_USER | PAL_ZERO);   // 동일 이유
    // swapping 이후 추가 필요.
    lock_acquire(&frame_lock);
    if (kaddr == NULL) {      // swapping으로 기존 frame_table 사용.
        new_frame = swap_out();       // 기존 frame 재활용.
    }
    else {      // 새 frame 만들고, frame_table에 저장.
        new_frame = calloc(1, sizeof(struct frame));
        new_frame->page = kaddr;
        list_push_back(&frame_table, &new_frame->frame_elem);
    }
    swap_in(new_frame, sp);
    new_frame->thread = tc;
    new_frame->sp = sp;

    // synchronize 하기
    // pagedir_set_page(tc->pagedir, sp->upage, new_frame->page, sp->writable);

    lock_release(&frame_lock);
    return new_frame->page;
}


void* free_frame(struct thread* tc, void *page) {
    struct list_elem* ft_elem = list_begin(&frame_table);
    for ( ;ft_elem != list_end(&frame_table); ) {
        struct frame* frame = list_entry(ft_elem, struct frame, frame_elem);
        if (frame->thread == tc && frame->page == page) {
            pagedir_clear_page(tc->pagedir, frame->sp->upage);     // 이거 안 하면 double free 일어남
            palloc_free_page(page);
            ft_elem = list_remove(ft_elem);
            free(frame);
            break;
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

struct frame* frame_evict() { // 죽일놈 선택하기만
    while(1){
        struct list_elem* frame_elem = list_begin(&frame_table); // 우리는 맨 앞만 본다, 앞에꺼 뺴서 맨 뒤에 넣기
        struct frame* frame = list_entry(frame_elem, struct frame, frame_elem);

        if (pagedir_is_accessed(frame->thread->pagedir, frame->sp->upage)) {    // give a second chance
            pagedir_set_accessed(frame->thread->pagedir, frame->sp->upage, false);      // 확인함
            // 맨 앞을 빼고, 맨뒤에 넣음.
            list_remove(frame_elem);
            list_push_back(&frame_table, frame_elem);
        }
        else {  // found frame to evict
            return frame;
        }


    }
    return NULL;
}