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

    bool dirty; //더티 빗
    bool access; // LRU 할라면, 접근 시간.
    struct hash_elem hash_elem;
    bool valid; // 유효한 페이지인지
};

struct supplemental_page* create_new_sp (struct file* file,
                                            int32_t ofs,
                                            uint8_t* upage,
                                            uint32_t read_bytes,
                                            uint32_t zero_bytes,
                                            bool writable);

unsigned hashing_func(struct hash_elem *he, void * aux);
bool hash_less_page(const struct hash_elem *a, const struct hash_elem *b, void * aux);
struct supplemental_page *spt_find_page(struct hash *spt, void *vaddr);
#endif