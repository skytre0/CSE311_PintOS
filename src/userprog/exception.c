#include "userprog/exception.h"
#include <inttypes.h>
#include <stdio.h>
#include "userprog/gdt.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

#include "vm/page.h"
#include "vm/frame.h"
#include "lib/kernel/hash.h"

#include "threads/palloc.h"
#include "userprog/process.h"

/* Number of page faults processed. */
static long long page_fault_cnt;

static void kill (struct intr_frame *);
static void page_fault (struct intr_frame *);

//
// void numexit(int num) {
//    // all child's sema up & mine down
//    while( list_begin( &(thread_current()->children) ) != list_end( &(thread_current()->children) ) ){
//      struct thread *t = list_entry(list_begin(&(thread_current()->children)), struct thread, am_child);
//      sema_up(&(t->withparent));
//      sema_down(&(thread_current()->withchild));
//    }
//    // 여기서 remove all my files
//    while( list_begin( &(thread_current()->fds) ) != list_end( &(thread_current()->fds) ) ){
//      struct filedata *cf = list_entry(list_begin( &(thread_current()->fds) ), struct filedata, fdselem);
//      numclose(cf->targetfd);
//    }
//    // my sema down
//    if(thread_current()->parent == NULL) thread_exit();
//    sema_down(&(thread_current()->withparent));
 
//    // parent의 children에서 본인 제거.
//    list_remove(&(thread_current()->am_child));
 
//    // parent's sema up
//    printf ("%s: exit(%d)\n", thread_name(), num);
//    sema_up(&(thread_current()->parent->withchild));
//    thread_exit();
//  }
//

/* Registers handlers for interrupts that can be caused by user
   programs.

   In a real Unix-like OS, most of these interrupts would be
   passed along to the user process in the form of signals, as
   described in [SV-386] 3-24 and 3-25, but we don't implement
   signals.  Instead, we'll make them simply kill the user
   process.

   Page faults are an exception.  Here they are treated the same
   way as other exceptions, but this will need to change to
   implement virtual memory.

   Refer to [IA32-v3a] section 5.15 "Exception and Interrupt
   Reference" for a description of each of these exceptions. */
void
exception_init (void) 
{
  /* These exceptions can be raised explicitly by a user program,
     e.g. via the INT, INT3, INTO, and BOUND instructions.  Thus,
     we set DPL==3, meaning that user programs are allowed to
     invoke them via these instructions. */
  intr_register_int (3, 3, INTR_ON, kill, "#BP Breakpoint Exception");
  intr_register_int (4, 3, INTR_ON, kill, "#OF Overflow Exception");
  intr_register_int (5, 3, INTR_ON, kill,
                     "#BR BOUND Range Exceeded Exception");

  /* These exceptions have DPL==0, preventing user processes from
     invoking them via the INT instruction.  They can still be
     caused indirectly, e.g. #DE can be caused by dividing by
     0.  */
  intr_register_int (0, 0, INTR_ON, kill, "#DE Divide Error");
  intr_register_int (1, 0, INTR_ON, kill, "#DB Debug Exception");
  intr_register_int (6, 0, INTR_ON, kill, "#UD Invalid Opcode Exception");
  intr_register_int (7, 0, INTR_ON, kill,
                     "#NM Device Not Available Exception");
  intr_register_int (11, 0, INTR_ON, kill, "#NP Segment Not Present");
  intr_register_int (12, 0, INTR_ON, kill, "#SS Stack Fault Exception");
  intr_register_int (13, 0, INTR_ON, kill, "#GP General Protection Exception");
  intr_register_int (16, 0, INTR_ON, kill, "#MF x87 FPU Floating-Point Error");
  intr_register_int (19, 0, INTR_ON, kill,
                     "#XF SIMD Floating-Point Exception");

  /* Most exceptions can be handled with interrupts turned on.
     We need to disable interrupts for page faults because the
     fault address is stored in CR2 and needs to be preserved. */
  intr_register_int (14, 0, INTR_OFF, page_fault, "#PF Page-Fault Exception");
}

/* Prints exception statistics. */
void
exception_print_stats (void) 
{
  printf ("Exception: %lld page faults\n", page_fault_cnt);
}


/* Handler for an exception (probably) caused by a user process. */
static void
kill (struct intr_frame *f) 
{
  /* This interrupt is one (probably) caused by a user process.
     For example, the process might have tried to access unmapped
     virtual memory (a page fault).  For now, we simply kill the
     user process.  Later, we'll want to handle page faults in
     the kernel.  Real Unix-like operating systems pass most
     exceptions back to the process via signals, but we don't
     implement them. */
     
  /* The interrupt frame's code segment value tells us where the
     exception originated. */
  switch (f->cs)
    {
    case SEL_UCSEG:
      /* User's code segment, so it's a user exception, as we
         expected.  Kill the user process.  */
      printf ("%s: dying due to interrupt %#04x (%s).\n",
              thread_name (), f->vec_no, intr_name (f->vec_no));
      intr_dump_frame (f);
      thread_exit (); 

    case SEL_KCSEG:
      /* Kernel's code segment, which indicates a kernel bug.
         Kernel code shouldn't throw exceptions.  (Page faults
         may cause kernel exceptions--but they shouldn't arrive
         here.)  Panic the kernel to make the point.  */
      intr_dump_frame (f);
      PANIC ("Kernel bug - unexpected interrupt in kernel"); 

    default:
      /* Some other code segment?  Shouldn't happen.  Panic the
         kernel. */
      printf ("Interrupt %#04x (%s) in unknown segment %04x\n",
             f->vec_no, intr_name (f->vec_no), f->cs);
      thread_exit ();
    }
}


void paging_simple(struct supplemental_page *sp, uint8_t *kpage);





/* Page fault handler.  This is a skeleton that must be filled in
   to implement virtual memory.  Some solutions to project 2 may
   also require modifying this code.

   At entry, the address that faulted is in CR2 (Control Register
   2) and information about the fault, formatted as described in
   the PF_* macros in exception.h, is in F's error_code member.  The
   example code here shows how to parse that information.  You
   can find more information about both of these in the
   description of "Interrupt 14--Page Fault Exception (#PF)" in
   [IA32-v3a] section 5.15 "Exception and Interrupt Reference". */
static void
page_fault (struct intr_frame *f) 
{
  bool not_present;  /* True: not-present page, false: writing r/o page. */
  bool write;        /* True: access was write, false: access was read. */
  bool user;         /* True: access by user, false: access by kernel. */
  void *fault_addr;  /* Fault address. */

  /* Obtain faulting address, the virtual address that was
     accessed to cause the fault.  It may point to code or to
     data.  It is not necessarily the address of the instruction
     that caused the fault (that's f->eip).
     See [IA32-v2a] "MOV--Move to/from Control Registers" and
     [IA32-v3a] 5.15 "Interrupt 14--Page Fault Exception
     (#PF)". */
  asm ("movl %%cr2, %0" : "=r" (fault_addr));

  /* Turn interrupts back on (they were only off so that we could
     be assured of reading CR2 before it changed). */
  intr_enable ();

  /* Count page faults. */
  page_fault_cnt++;

  /* Determine cause. */
  not_present = (f->error_code & PF_P) == 0;
  write = (f->error_code & PF_W) != 0;
  user = (f->error_code & PF_U) != 0;

  /* To implement virtual memory, delete the rest of the function
     body, and replace it with code that brings in the page to
     which fault_addr refers. */

// 일단 널이랑 커널은 쳐내
   if( fault_addr == NULL || is_kernel_vaddr(fault_addr) ) {
      numexit(-1);
   }

   // int32_t* page_vaddr = pg_round_down(fault_addr); // 페이지 단위로 정렬
   //  printf("[PAGE_FAULT] Fault occurred for vaddr: %p\n", page_vaddr); // 디버깅용

   // 폴트인데 유효성 판단 해야함. 이제 보조 테이블이 필요함.
   struct supplemental_page *sp = spt_find_page(&thread_current()->spt, fault_addr);
   uint8_t *kpage;

   if ( sp == NULL){    // file이 아님 = stack을 연장해야 함.
      printf("in the stack growth area 1\n");

      // fault_addr >= f->esp - 32); 32 안에 있으면 스택키우는 거임.
      bool is_stack_growth = (fault_addr >= f->esp - 32 && PHYS_BASE - fault_addr <= (1<<23)); // 8mb 보고
      if (!is_stack_growth) {
         numexit(-1);
      }
      // 스택 키우기
      printf("in the stack growth area 2\n");

      uint8_t *upage = pg_round_down(fault_addr);
      struct supplemental_page* new_sp = create_new_sp(NULL, NULL, upage, 0, PGSIZE, true);
      hash_insert(&thread_current()->spt, &new_sp->hash_elem);
      kpage = stack_frame_alloc(thread_current()); // 스택 할당하기

      if (kpage == NULL)
         numexit(-1);
      // 페이징 할당하기
      // 매핑하기
   }
// printf("===============1st thread in page fault : %d===================\n", thread_current()->tid);
   // struct hash_elem *hash_find (struct hash *, struct hash_elem *);

   // 유효한 경우: 단순히 디스크 등에서 메모리로 아직 안 올라온 페이지

         /* Get a page of memory. */

   else {      // file에 있는 page 발견
      // 이제는 ofs 만큼 가서 읽어야 함.
      file_seek (sp->file, sp->ofs);
      kpage = file_frame_alloc(thread_current());

      if (kpage == NULL)
         numexit(-1);

      paging_simple(sp, kpage);     // paging according to read_byte / zero_byte size (PGSIZE)
   }

   bool success = pagedir_set_page(thread_current()->pagedir, sp->upage, kpage, sp->writable);
   if(!success) {
      palloc_free_page(kpage);
      numexit(-1);
   }

   /* Load this page. */
   // if (file_read (sp->file, kpage, sp->read_bytes) != (int) sp->read_bytes)
   //    {
   //       palloc_free_page (kpage);
   //       numexit(-1);
   //    }
   // memset (kpage + sp->read_bytes, 0, sp->zero_bytes);

   // bool success = pagedir_set_page(thread_current()->pagedir, sp->upage, kpage, sp->writable);
   // if(!success) {
   //  palloc_free_page(kpage);
   //  numexit(-1);
   // }


   /* Add the page to the process's address space. */
  //  if (!(pagedir_get_page (thread_current()->pagedir, sp->upage) == NULL)) {
  //     if (!(pagedir_set_page (thread_current()->pagedir, sp->upage, kpage, sp->writable))) {
  //        palloc_free_page (kpage);
  //        return false; 

  //     }
  //  }
   // if (!(pagedir_get_page (thread_current()->pagedir, sp->upage) == NULL
   //        && pagedir_set_page (thread_current()->pagedir, sp->upage, kpage, sp->writable))) 
   //    {
   //       palloc_free_page (kpage);
   //       return false; 
   //    }
// printf("===============2nd thread in page fault : %d===================\n", thread_current()->tid);

// spt 가지고 로드하면 됌.

// 빈 프레임 확보: 물리 메모리에서 비어있는 공간(프레임)을 찾습니다. 만약 없다면, 기존에 사용 중인 프레임 중 하나를 비웁니다 (페이지 교체 알고리즘 사용)
// 데이터 로딩: 필요한 페이지 데이터를 디스크(파일 시스템 또는 스왑 영역)에서 2번에서 확보한 프레임으로 읽어옵니다.
// 페이지 테이블 갱신: 해당 가상 주소가 방금 데이터를 로드한 물리 프레임을 가리키도록 페이지 테이블을 수정합니다.
// 명령 재시작: 폴트를 발생시켰던 명령어를 다시 실행합니다. 이제는 메모리에 데이터가 있으므로 정상적으로 수행됩니다.

  // if( user ) numexit(-1);
  // printf ("Page fault at %p: %s error %s page in %s context.\n",
  //         fault_addr,
  //         not_present ? "not present" : "rights violation",
  //         write ? "writing" : "reading",
  //         user ? "user" : "kernel");
  // kill (f);
}

void paging_simple(struct supplemental_page *sp, uint8_t *kpage) {
   if (sp->read_bytes == PGSIZE) {
      if (file_read (sp->file, kpage, sp->read_bytes) != (int) sp->read_bytes)
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