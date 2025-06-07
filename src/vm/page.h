#include "hash.h"

struct supplemental_page{
    void* addr; // 주소
    bool dirty; //더티 빗
    uint8_t access_time; // LRU 할라면, 접근 시간.
    struct hash_elem hash_elem;
    bool valid; // 유효한 페이지인지
};