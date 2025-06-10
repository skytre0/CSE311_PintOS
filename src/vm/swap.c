#include "vm/swap.h"

//threads/init.c 에다가 박아서 1번만 호출되게 함.
void swap_init(void){
    swap_block = block_get_role (BLOCK_SWAP);

    if (swap_block == NULL) { PANIC("Swap block device not found.");  }// 시스템을 멈추고 메시지 출력

    size_t slot_cnt = block_size(swap_block) / (PGSIZE/BLOCK_SECTOR_SIZE); // 이러면 1페이지가 8섹터로 리턴되는거 보정해서 페이지 개수로
    swap_bitmap = bitmap_create(slot_cnt); // bitmap_create는 모든 비트를 0(사용 가능) -> block_idx / 8

    if (swap_bitmap == NULL) { PANIC("Swap bitmap creation failed."); } // 디버깅용.

    lock_init (&swap_lock); // 동기화 문제 1번에 1놈만 되게
}



// swap_in
void swap_in(struct frame* new_frame, struct supplemental_page* sp) {
    lock_acquire(&swap_lock);
    if (sp->in_swap == true) {      // swap에 있음 = 그냥 file이나 stack.
        size_t swap_index = sp->swap_pos;
        sp->in_swap = false;
        sp->swap_pos = -1;

        // swap block에서 내용 읽어오기.
        int i = 0;
        for (i = 0; i < (PGSIZE / BLOCK_SECTOR_SIZE); i++) {        // block_write와 같은 방식(block 단위)
            block_read(swap_block, swap_index * (PGSIZE / BLOCK_SECTOR_SIZE) + i, new_frame->page + i * BLOCK_SECTOR_SIZE); // 메모리로 슛
        }
        bitmap_flip (swap_bitmap, swap_index);        
    }
    else {      // swap에 없음 = mmap / 처음 읽는 것 = in_swap, swap_pos 바꿀 거 없음.
        paging_simple(sp, new_frame->page);     // paging according to read_byte / zero_byte size (PGSIZE)
    }

    lock_release(&swap_lock);
}

// swap_out
struct frame* swap_out() {
    lock_acquire(&swap_lock);
    // victim인 frame 찾기
    struct frame* frame = frame_evict();

    if (pagedir_is_dirty(frame->thread->pagedir, frame->sp->upage)) {   // something written = save to swap
        // 비어있는 공간 찾기
        pagedir_set_dirty(frame->thread->pagedir, frame->sp->upage, false);
        if (frame->sp->from_where > 0) {   // is mmap = save to file = 그냥 나중에 file에서 다시 가져오는 거라서 in_swap 변경할 필요 없음.
            sema_down(&filesema);
            file_write_at(frame->sp->file, frame->page, frame->sp->read_bytes, frame->sp->ofs);   // nummunmap하면 frame 포함 전부 날려서 못 씀.
            sema_up(&filesema);
        }
        else {      // need to swap
            size_t swap_index = bitmap_scan_and_flip (swap_bitmap, 0, 1, false);
            frame->sp->in_swap = true;
            frame->sp->swap_pos = swap_index;
            int i = 0;
            for (i = 0; i < (PGSIZE / BLOCK_SECTOR_SIZE); i++) {        // swap_index에 1/8만큼 작성 가능해서, 1 page = 0~7, 2page = 8~15 ... 라서 쓸 위치 이렇게 작성함.
                block_write (swap_block, swap_index * (PGSIZE / BLOCK_SECTOR_SIZE) + i, frame->page + i * BLOCK_SECTOR_SIZE); // block_write(스왑디바이스, 쓸 위치, 데이터소스 주소)
            }
        }
    }
    else {      // nothing written = don't save to swap = just overwrite

    }
    pagedir_clear_page(frame->thread->pagedir, frame->sp->upage);
    lock_release(&swap_lock); // 풀어주기
    return frame;
}

void rm_swap(struct supplemental_page* sp) {
    bitmap_flip (swap_bitmap, sp->swap_pos);   
}