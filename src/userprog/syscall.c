#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"

#include "threads/vaddr.h"
#include "vm/page.h"

static void syscall_handler (struct intr_frame *);
struct semaphore filesema;

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  sema_init(&filesema, 1);    // will be used as lock
}

// enum 
//   {
//     /* Projects 2 and later. */
//     SYS_HALT,                   /* Halt the operating system. */
//     SYS_EXIT,                   /* Terminate this process. */
//     SYS_EXEC,                   /* Start another process. */
//     SYS_WAIT,                   /* Wait for a child process to die. */
//     SYS_CREATE,                 /* Create a file. */
//     SYS_REMOVE,                 /* Delete a file. */
//     SYS_OPEN,                   /* Open a file. */
//     SYS_FILESIZE,               /* Obtain a file's size. */
//     SYS_READ,                   /* Read from a file. */
//     SYS_WRITE,                  /* Write to a file. */
//     SYS_SEEK,                   /* Change position in a file. */
//     SYS_TELL,                   /* Report current position in a file. */
//     SYS_CLOSE,                  /* Close a file. */

//     /* Project 3 and optionally project 4. */
//     SYS_MMAP,                   /* Map a file into memory. */
//     SYS_MUNMAP,                 /* Remove a memory mapping. */

//     /* Project 4 only. */
//     SYS_CHDIR,                  /* Change the current directory. */
//     SYS_MKDIR,                  /* Create a directory. */
//     SYS_READDIR,                /* Reads a directory entry. */
//     SYS_ISDIR,                  /* Tests if a fd represents a directory. */
//     SYS_INUMBER                 /* Returns the inode number for a fd. */
//   };


// switch case 문으로 경우에 대해서 분기
// 44 page 에 인자 꺼내는 법 써져 있음
// system call 구현

void numhalt(void);
void numexit(int);
int numexec(const char* cmd_line);
int numwait(int);
bool numcreate(const char* createname, unsigned createsize);
bool numremove(const char* removename);
int numopen(const char* openname);
int numfilesize(int sizefd);
int numread(int readfd, void* readbuffer, unsigned readsize);
int numwrite(int writefd, void* writebuffer, unsigned writesize);
void numseek(int seekfd, unsigned seekpos);
unsigned numtell(int tellfd);
void numclose(int closefd);

// project 3
int nummmap(int fd, void* addr);
void nummunmap(int mapping);



bool check_user_mem(void* addr, int addrsize, bool is_name) {
  void* i;
  for (i = addr; i < addr + addrsize; i++) {
    if (i == NULL) return false;
    if (!is_user_vaddr(i)) return false;
    if ((pagedir_get_page(thread_current()->pagedir, i) == NULL) && 
        (spt_find_page(&thread_current()->spt, i) == NULL)) return false;
    if (is_name)
      if (*(char *)(i) == NULL) break;
  }
  return true;
}

void buffer_check(void* addr, int addrsize) {
  void* check;
  for (check = addr; check < addr + addrsize; check += PGSIZE) {
    struct supplemental_page* sp = spt_find_page(&thread_current()->spt, check);
    if (!sp->writable)  numexit(-1);
  }
}


static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  // printf ("system call!\n");
  // int tmp=f->esp;
  // while(tmp+4<= 0xc0000000){
  //   printf("Address: %8x    Data: %8x\n", tmp, *(int*)tmp);  
  //   tmp+=4;
  // }

  // user process의 syscall
  if(!check_user_mem(f->esp, 4, 0)) numexit(-1);
  int number = *(int*)(f->esp);
  

  switch (number)
  {
    case SYS_HALT:
      // printf("called sys_halt\n");
      numhalt();
      return;


    case SYS_EXIT:
      // printf("called sys_exit\n");
      if(!check_user_mem(f->esp+4, 4, 0)) numexit(-1);
      f->eax = *(int*)(f->esp + 4); //return status
      thread_current()->exitval = *(int*)(f->esp + 4);
      numexit(*(int*)(f->esp + 4));
      return;

      
    case SYS_EXEC:
      if(!check_user_mem(f->esp+4, 4, 0)) numexit(-1);
      const char *cmd_line = *(int*)(f->esp + 4);
      f->eax = numexec(cmd_line);
      return;


    case SYS_WAIT:
      if(!check_user_mem(f->esp+4, 4, 0)) numexit(-1);
      int waitfd = *(int*)(f->esp + 4);
      f->eax = numwait(waitfd);
      return;


    case SYS_CREATE:
      // printf("called sys_create\n");
      if(!check_user_mem(f->esp+4, 4, 0)) numexit(-1);
      const char* createname = *(int*)(f->esp + 4); 
      if(!check_user_mem(f->esp+8, 4, 0)) numexit(-1);
      int32_t initial_size = *(int32_t*)(f->esp + 8);
      f->eax = numcreate(createname, initial_size);
      return;


    case SYS_REMOVE:
      if(!check_user_mem(f->esp+4, 4, 0)) numexit(-1);
      const char* removename = *(int*)(f->esp + 4);
      f->eax = numremove(removename);
      return;


    case SYS_OPEN:;
      // printf("called sys_open\n");
      if(!check_user_mem(f->esp+4, 4, 0)) numexit(-1);
      const char *openname = *(int*)(f->esp + 4);
      f->eax = numopen(openname);
      return;


    case SYS_FILESIZE:
      if(!check_user_mem(f->esp+4, 4, 0)) numexit(-1);
      int sizefd = *(int*)(f->esp + 4);
      f->eax = numfilesize(sizefd);
      return;

      
    case SYS_READ:
      if(!check_user_mem(f->esp+4, 4, 0)) numexit(-1);
      int readfd = *(int*)(f->esp + 4);
      if(!check_user_mem(f->esp+8, 4, 0)) numexit(-1);
      void* readbuffer = *(int*)(f->esp + 8);
      if(!check_user_mem(f->esp+12, 4, 0)) numexit(-1);
      unsigned int readsize = *(unsigned int*)(f->esp + 12);
      f->eax = numread(readfd, readbuffer, readsize);
		  return;


    case SYS_WRITE: ;
      // printf("called sys_write\n");
      if(!check_user_mem(f->esp+4, 4, 0)) numexit(-1);
      int writefd = *(int*)(f->esp + 4);
      if(!check_user_mem(f->esp+8, 4, 0)) numexit(-1);
      void* writebuffer = *(int*)(f->esp + 8);
      if(!check_user_mem(f->esp+12, 4, 0)) numexit(-1);
      unsigned writesize = *(unsigned*)(f->esp + 12);
      // printf("buffer : %x\n", buffer);
      // printf("size : %d\n", writesize);
      f->eax = numwrite(writefd, writebuffer, writesize);
      return;


    case SYS_SEEK:
      if(!check_user_mem(f->esp+4, 4, 0)) numexit(-1);
      int seekfd = *(int*)(f->esp + 4);
      if(!check_user_mem(f->esp+8, 4, 0)) numexit(-1);
      int32_t seekpos = *(int*)(f->esp + 8);
      numseek(seekfd, seekpos);
      return;


    case SYS_TELL:
      if(!check_user_mem(f->esp+4, 4, 0)) numexit(-1);
      int tellfd = *(int*)(f->esp + 4);
      f->eax = numtell(tellfd);
      return;


    case SYS_CLOSE: ;
      // printf("called sys_close\n");
      if(!check_user_mem(f->esp+4, 4, 0)) numexit(-1);
      int closefd = *(int*)(f->esp + 4);
      numclose(closefd);
      return;

    
    // /* Project 3 and optionally project 4. */
    case SYS_MMAP:  ;
      if(!check_user_mem(f->esp+4, 4, 0)) numexit(-1);
      int mmapfd = *(int*)(f->esp + 4);
      if(!check_user_mem(f->esp+8, 4, 0)) numexit(-1);
      void* mmapaddr = *(int*)(f->esp + 8);
      f->eax = nummmap(mmapfd, mmapaddr);
      return;

    case SYS_MUNMAP: ;
      if(!check_user_mem(f->esp+4, 4, 0)) numexit(-1);
      int mapping = *(int*)(f->esp + 4);
      nummunmap(mapping);
      return;
    
    // /* Project 4 only. */
    // case SYS_CHDIR:
    //     break;
    // case SYS_MKDIR:
    //     break;
    // case SYS_READDIR:
    //     break;
    // case SYS_ISDIR:
    //     break;
    // case SYS_INUMBER:
    //     break;
    
    /* code */
    // break;
  
  default:
    numexit(-1);
    shutdown_power_off();
    return;
  } 

  return;
}


void numhalt(void) {
  shutdown_power_off();
  return;
}


void numexit(int num) {
  thread_current()->exitval = num;
  // all child's sema up & mine down
  while( list_begin( &(thread_current()->children) ) != list_end( &(thread_current()->children) ) ){
    struct thread *t = list_entry(list_begin(&(thread_current()->children)), struct thread, am_child);
    sema_up(&(t->withparent));
    sema_down(&(thread_current()->withchild));
  }
  // 여기서 remove all my files
  while( list_begin( &(thread_current()->fds) ) != list_end( &(thread_current()->fds) ) ){
    struct filedata *cf = list_entry(list_begin( &(thread_current()->fds) ), struct filedata, fdselem);
    numclose(cf->targetfd);
  }
  // my sema down
  if(thread_current()->parent == NULL) thread_exit();
  sema_down(&(thread_current()->withparent));

  // parent의 children에서 본인 제거.
  list_remove(&(thread_current()->am_child));
  if (thread_current()->execfile != NULL) file_allow_write(thread_current()->execfile);
  file_close(thread_current()->execfile);



  // project 3로 spt, frame 등 모두 free
  // spt 순회하면서 검사, MMAP이면 nummunmap 호출, 아니면 frame에 있는지 확인, 있으면 free_frame -> sp free
  while( list_begin( &(thread_current()->mmaps) ) != list_end( &(thread_current()->mmaps) ) ){
    struct mapinfo* mi = list_entry(list_begin(&(thread_current()->mmaps)), struct mapinfo, mmap_elem);
    nummunmap(mi->mapid);
  }

  // 이제 남은 거 무조건 MMAP 아닌 것들만 spt에 남아 있음 -> hash_destroy가 알아서 hash에서 지우고, free_hash_elem이 알아서 frame, sp 지움.
  hash_destroy(&thread_current()->spt, free_hash_elem);
  


  // parent's sema up
  printf ("%s: exit(%d)\n", thread_name(), num);
  sema_up(&(thread_current()->parent->withchild));
  sema_down(& ( thread_current()->withparent) );
  thread_exit();
}


int numexec(const char* cmd_line) {
  if( ! check_user_mem(cmd_line, PGSIZE, 1) ) numexit(-1);
  return process_execute(cmd_line);
}


int numwait(int num) {
  return process_wait(num);
}


bool numcreate(const char* createname, unsigned createsize) {
  if( !check_user_mem(createname, 14, 1) ) numexit(-1);
  sema_down(&filesema);
  bool retval = filesys_create (createname, createsize);
  sema_up(&filesema);
  return retval;
}


bool numremove(const char* removename) {
  if( ! check_user_mem(removename, 14, 1) ) numexit(-1);
  sema_down(&filesema);
  bool retval = filesys_remove (removename);
  sema_up(&filesema);
  return retval;
}


int numopen(const char* openname) {
  if( ! check_user_mem(openname, 14, 1) ) numexit(-1);
  sema_down(&filesema);
  struct file* actualfile = filesys_open (openname);
  sema_up(&filesema);
  if(actualfile == NULL) return -1;

  struct filedata* openfile = calloc(1, sizeof(struct filedata));
  openfile->targetfd = thread_current()->availablefd;
  openfile->targetfile = actualfile;
  openfile->targetname = calloc(1, strlen(openname) + 1);
  strlcpy(openfile->targetname, openname, strlen(openname) + 1);\
  list_push_back(&(thread_current()->fds), &(openfile->fdselem));
  return thread_current()->availablefd++;
}


int numfilesize(int sizefd) {
  struct filedata* sizefile = find_file(sizefd);
  if (sizefile == NULL) numexit(-1);
  sema_down(&filesema);
  int retval = file_length(sizefile->targetfile);
  sema_up(&filesema);
  return retval;
}


int numread(int readfd, void* readbuffer, unsigned readsize) {
  if (readfd == 0) {
    int i;
    sema_down(&filesema);
    for (i = 0; i < readsize; i++)
      *((char *)readbuffer++) = input_getc();
    sema_up(&filesema);
    return readsize;
  }
  else if (readfd > 1) {
    struct filedata* readfile = find_file(readfd);
    if (readfile == NULL) return -1;
    if(!check_user_mem(readbuffer, readsize, 0)) numexit(-1);
    // in project 3, testcase check whether buffer address valid or not -> read syscall write on buffer
    buffer_check(readbuffer, readsize);
    sema_down(&filesema);
    int retval = file_read (readfile->targetfile, readbuffer, readsize);
    sema_up(&filesema);
    return retval;
  }
  else return -1;
}


int numwrite(int writefd, void* writebuffer, unsigned writesize) {
  if (writefd == 1) {
    if( !check_user_mem(writebuffer, writesize, 0) ) numexit(-1);
    sema_down(&filesema);
    putbuf(writebuffer, writesize);
    sema_up(&filesema);
    return writesize;
  }
  else if (writefd > 0) {
    // check if writing unavilable
    struct filedata* writefile = find_file(writefd);
    if (writefd < 1 || writefile == NULL || writefile->targetfile == thread_current()->execfile) return 0;
    if( ! check_user_mem(writebuffer, writesize, 0) ) numexit(-1);
    
    // can write less or equal to writesize
    sema_down(&filesema);
    int retval = file_write(writefile->targetfile, writebuffer, writesize);
    sema_up(&filesema);
    return retval;
  }
  else
    return -1;
  return;
}


void numseek(int seekfd, unsigned seekpos) {
  struct filedata* seekfile = find_file(seekfd);
  if (seekfile == NULL) return;
  sema_down(&filesema);
  file_seek(seekfile->targetfile, seekpos);
  sema_up(&filesema);
  return;
}


unsigned numtell(int tellfd) {
  struct filedata* tellfile = find_file(tellfd);
  if (tellfile == NULL) numexit(-1);
  sema_down(&filesema);
  int retval = (int)file_tell(tellfile->targetfile);
  sema_up(&filesema);
  return retval;
}


void numclose(int closefd) {
  struct filedata* closefile = find_file(closefd);
  if (closefd < 2 || closefile == NULL) numexit(-1);
  sema_down(&filesema);
  file_close(closefile->targetfile);
  sema_up(&filesema);
  list_remove(&(closefile->fdselem));
  free(closefile->targetname);
  free(closefile);
  return;
}


int nummmap(int fd, void* addr) {
  // validate cond to fail
  // page 단위 검사 여부는 보류 -> 일단 시작과 끝만.
  if ((fd == 0 || fd == 1) || numfilesize(fd) == 0 || addr == 0 || (int)addr % PGSIZE != 0)  return -1;
  void *uaddr = addr;
  int limit = numfilesize(fd);
  for ( ; uaddr < addr + limit; uaddr += PGSIZE) {
    if (spt_find_page(&thread_current()->spt, uaddr) != NULL)   return -1;
  }

  struct file* mmapfile = (find_file(fd))->targetfile;
  sema_down(&filesema);
  mmapfile = file_reopen(mmapfile);
  sema_up(&filesema);

  // mmap할 파일에 정보 기록
  struct mapinfo* newm = calloc(1, sizeof(struct mapinfo));
  newm->vaddr = addr;
  newm->fd = fd;
  newm->file = mmapfile;
  newm->mapid = thread_current()->mapid++;
  newm->pagenum = (uaddr - addr) / PGSIZE;

  list_push_back(&thread_current()->mmaps, &newm->mmap_elem);
  int ofs = 0;
  for (uaddr = addr; uaddr < addr + limit; uaddr += PGSIZE) {
      int read_bytes = (limit - ofs < PGSIZE) ? (limit - ofs) : PGSIZE;
      int zero_bytes = PGSIZE - read_bytes;

      // mapid 바로 기록 -> nummunmap 바로 가능해짐
      struct supplemental_page* new_sp = create_new_sp(mmapfile,
                                                        ofs,
                                                        uaddr,
                                                        read_bytes,
                                                        zero_bytes,
                                                        true, newm->mapid);
      ofs += PGSIZE;
      hash_insert(&thread_current()->spt, &new_sp->hash_elem);
  }

  return newm->mapid;
}


void nummunmap(int mapping) {
  struct mapinfo* mapfile = find_mapfile(mapping);
  if (mapfile == NULL)  numexit(-1);
  int i = 0;
  for ( ; i < mapfile->pagenum; i++) {
    struct supplemental_page* sp = spt_find_page(&thread_current()->spt, mapfile->vaddr + (PGSIZE * i));
    void* kaddr = pagedir_get_page(thread_current()->pagedir, sp->upage);
    if (kaddr != NULL) {    // frame에 있음 = palloc_free_page 해야 함
      if (pagedir_is_dirty(thread_current()->pagedir, sp->upage)) {   // 내용 복사해야 함.
        sema_down(&filesema);
        file_write_at(sp->file, kaddr, sp->read_bytes, sp->ofs);   // file에, kpage의 내용을, read_bytes만큼, pg_round_down(ofs)부터 작성해라.
        sema_up(&filesema);
      }
      // 해당 frame 찾고 free해야 함.
      free_frame(thread_current(), kaddr);
    }
    else {} // frame에 없음 = eviction 당해서 반영된 상태든지, 애초에 mmap만 하고 사용한 적 없음
    // spt에서 제거 & mmaps에서도 제거 -> 본인 spt 제거
    hash_delete(&thread_current()->spt, &sp->hash_elem);
    list_remove(&mapfile->mmap_elem);
    free(sp);
  }
  // reopen 제거 -> 이거 syscall numopen으로 한 거 아니라 file_reopen으로 한 거라서 syscall numclose 대신 이거 씀
  sema_down(&filesema);
  file_close(mapfile->file);
  sema_up(&filesema);
  list_remove(&mapfile->mmap_elem);
  free(mapfile);
  return;
}