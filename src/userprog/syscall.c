#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"

#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);
static struct semaphore filesema;

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

bool check_user_mem(void* addr, int addrsize, bool is_name) {
  void* i;
  for (i = addr; i < addr + addrsize; i++) {
    if (i == NULL) return false;
    if (!is_user_vaddr(i)) return false;
    if (pagedir_get_page(thread_current()->pagedir, i) == NULL) return false;
    if (is_name)
      if (*(char *)(i) == NULL) break;
  }
  return true;
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
    // case SYS_MMAP:
    //     break;
    // case SYS_MUNMAP:
    //     break;
    
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
  // struct list_elem *removeele;
  // struct list* name_search_list = &(thread_current()->fds);

  // for (removeele = list_begin(name_search_list); removeele != list_end(name_search_list); removeele = list_next(removeele)) {
  //   struct filedata *removefile = list_entry(removeele, struct filedata, fdselem);
  //   if (strcmp(removefile->targetname, removename) == 0) {
  //     removefile->targetname = NULL;
  //     return filesys_remove (removename);
  //     list_remove(&(removefile->fdselem));    // resource issue로 일단 지움
  //     free(removefile->targetname);
  //     free(removefile);
  //     break;
  //   }
  // }
  // return false;
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
    numexit(-1);
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