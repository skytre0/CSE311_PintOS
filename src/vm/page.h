#ifndef PAGE_H
#define PAGE_H

#include "hash.h"
#include "../userprog/process.h"


struct supplemental_page{
    // load_segment   
    struct file* file;
    int32_t ofs;
    uint8_t* upage;
    uint32_t read_bytes;
    uint32_t zero_bytes;
    bool writable;
    int from_where;             // 음수 = file / stack = swap 대상, 양수 = mmap의 mapid
    struct hash_elem hash_elem;
    // 이 두개는 바뀔 수 있음 -> swap에 따라서
    bool in_swap;           // false = not in swap = 그냥 frame에 적으면 됨. true = swap에서 가져와야 함.
    int swap_pos;

    // bool dirty; //더티 빗
    // bool access; // LRU 할라면, 접근 시간.
    // bool valid; // 유효한 페이지인지
};

struct supplemental_page* create_new_sp (struct file* file,
                                            int32_t ofs,
                                            uint8_t* upage,
                                            uint32_t read_bytes,
                                            uint32_t zero_bytes,
                                            bool writable, int pt);

unsigned hashing_func(struct hash_elem *he, void * aux);
bool hash_less_page(const struct hash_elem *a, const struct hash_elem *b, void * aux);
struct supplemental_page *spt_find_page(struct hash *spt, void *vaddr);
void free_hash_elem(struct hash_elem *he, void * aux);
#endif