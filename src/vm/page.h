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

    void* addr; // 주소
    bool dirty; //더티 빗
    uint8_t access_time; // LRU 할라면, 접근 시간.
    struct hash_elem hash_elem;
    bool valid; // 유효한 페이지인지
};

struct supplemental_page* create_new_sp (struct file* file,
                                            int32_t ofs,
                                            uint8_t* upage,
                                            uint32_t read_bytes,
                                            uint32_t zero_bytes,
                                            bool writable);
