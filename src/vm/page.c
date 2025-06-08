#include "page.h"
#include "../threads/vaddr.h"

struct supplemental_page* create_new_sp (struct file* file, int32_t ofs, uint8_t* upage, 
                                        uint32_t read_bytes, uint32_t zero_bytes, bool writable, enum page_type pt) {
    struct supplemental_page* new_sp = calloc(1, sizeof(struct supplemental_page));
    new_sp->file = file;
    new_sp->ofs = ofs;
    new_sp->upage = upage;
    new_sp->read_bytes = read_bytes;
    new_sp->zero_bytes = zero_bytes;
    new_sp->writable = writable;
    new_sp->from_where = pt;
    return new_sp;
}

unsigned hashing_func (struct hash_elem *he, void *aux) {
  struct supplemental_page *sp = hash_entry (he, struct supplemental_page, hash_elem);

  return hash_bytes (&sp->upage, sizeof (sp->upage));
}

bool hash_less_page(const struct hash_elem *a, const struct hash_elem *b, void * aux){
  struct supplemental_page *sa = hash_entry(a, struct supplemental_page, hash_elem);
  struct supplemental_page *sb = hash_entry(b, struct supplemental_page, hash_elem);

  return sa->upage < sb->upage;
}


// spt 찾아주는 함수
struct supplemental_page *spt_find_page(struct hash *spt, void *vaddr) {
    struct supplemental_page tmp_sp;
    struct hash_elem *e;

    tmp_sp.upage = pg_round_down(vaddr);

    e = hash_find(spt, &tmp_sp.hash_elem);

    if (e != NULL) return hash_entry(e, struct supplemental_page, hash_elem);
    
    return NULL;
}
