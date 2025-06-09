#include "page.h"
#include "../threads/vaddr.h"

struct supplemental_page* create_new_sp (struct file* file, int32_t ofs, uint8_t* upage, 
                                        uint32_t read_bytes, uint32_t zero_bytes, bool writable, int pt) {
    struct supplemental_page* new_sp = calloc(1, sizeof(struct supplemental_page));
    new_sp->file = file;
    new_sp->ofs = ofs;
    new_sp->upage = upage;
    new_sp->read_bytes = read_bytes;
    new_sp->zero_bytes = zero_bytes;
    new_sp->writable = writable;
    
    new_sp->from_where = pt;
    new_sp->in_swap = false;
    new_sp->swap_pos = -1;
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


void free_hash_elem(struct hash_elem *he, void * aux) {
  struct supplemental_page *sp = hash_entry (he, struct supplemental_page, hash_elem);
  void* kaddr = pagedir_get_page(thread_current()->pagedir, sp->upage);
  if (kaddr != NULL) {    // frame에 있음
    free_frame(thread_current(), kaddr);
  }
  else if (sp->in_swap) {  // frame에 없고, eviction 당해서 swap된 상태
    // swapping 구현 이후에 구현 필요.
    rm_swap(sp);
  }
  // spt에서 제거 & 본인 spt 제거
  free(sp);
}

void paging_simple(struct supplemental_page *sp, uint8_t *kpage) {
   if (sp->read_bytes == PGSIZE) {
      if (file_read (sp->file, kpage, sp->read_bytes) != (int) sp->read_bytes)      // file seek로 원하는 위치에 현재 있음 = file_read_at 안 해도 됨.
         {
            palloc_free_page (kpage);
            numexit(-1);
         }
   }

   else if (sp->zero_bytes == PGSIZE) {
      memset (kpage, 0, sp->zero_bytes);
   }

   else {
      if (file_read (sp->file, kpage, sp->read_bytes) != (int) sp->read_bytes)
         {
            palloc_free_page (kpage);
            numexit(-1);
         }
      memset (kpage + sp->read_bytes, 0, sp->zero_bytes);
   }
   return;
}