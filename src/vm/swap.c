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
    // 스왑 in 구현해야함
    int i = 0;
    for (i = 0; i < (PGSIZE / BLOCK_SECTOR_SIZE); i++) {
        block_read(swap_block, swap_index * (PGSIZE / BLOCK_SECTOR_SIZE), kaddr); // 메모리로 슛
    }

    bitmap_set(swap_bitmap, swap_index, false);
    lock_release(&swap_lock);
}

// swap_out
size_t swap_out(void *kaddr) {
    lock_acquire(&swap_lock);
    // 비어있는 공간 찾기
    size_t swap_index = bitmap_scan_and_flip (swap_bitmap, 0, 1, false);
    int i = 0;
    for (i = 0; i < (PGSIZE / BLOCK_SECTOR_SIZE); i++) {
        block_write (swap_block, swap_index * (PGSIZE / BLOCK_SECTOR_SIZE) + i, kaddr + i * BLOCK_SECTOR_SIZE); // block_write(스왑디바이스, 쓸 위치, 데이터소스 주소)
    }
    lock_release(&swap_lock); // 풀어주기
    return swap_index;
}