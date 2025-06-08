#include "vm/swap.h"

//threads/init.c 에다가 박아서 1번만 호출되게 함.
void swap_init(void){
    swap_block = block_get_role (BLOCK_SWAP);
    size_t slot_cnt = block_size(swap_block) / (PGSIZE/BLOCK_SECTOR_SIZE); // 이러면 1페이지가 8섹터로 리턴되는거 보정해서 페이지 개수로
    swap_bitmap = bitmap_create(slot_cnt); //bitmap_create는 모든 비트를 0(사용 가능)
    lock_init (&swap_lock); // 동기화 문제 1번에 1놈만 되게
}



// swap_in
void swap_in(void *kaddr, size_t swap_index) {
    lock_acquire(&swap_lock);
    // TODO: Implement swap_in
    bitmap_set(swap_bitmap, swap_index, false);
    lock_release(&swap_lock);
}

// swap_out
void swap_out(void *kaddr) {
    lock_acquire(&swap_lock);
    // TODO: Implement swap_out
    lock_release(&swap_lock);
}