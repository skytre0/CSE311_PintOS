#include "page.h"

struct supplemental_page* create_new_sp (struct file* file, int32_t ofs, uint8_t* upage, 
                                        uint32_t read_bytes, uint32_t zero_bytes, bool writable) {
    struct supplemental_page* new_sp = calloc(1, sizeof(struct supplemental_page));
    new_sp->file = file;
    new_sp->ofs = ofs;
    new_sp->upage = upage;
    new_sp->read_bytes = read_bytes;
    new_sp->zero_bytes = zero_bytes;
    new_sp->writable = writable;
    return new_sp;
}