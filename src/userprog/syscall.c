#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"

#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
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


#define EXIT \
        ({   \
          f->esp = PHYS_BASE -8;\
          *(int*)(f->esp) = SYS_EXIT;\
          *(int*)(f->esp + 4) = -1;\
          syscall_handler(f);\
        })

// switch case 문으로 경우에 대해서 분기
// 44 page 에 인자 꺼내는 법 써져 있음
// system call 구현

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
  int number;
  int len=0;
  if(!check_user_mem(f->esp, 4, 0)) EXIT;
  number = *(int*)(f->esp);
  
  

  switch (number)
  {
    case SYS_HALT:
      // printf("called sys_halt\n");
      shutdown_power_off();
      return;


      case SYS_EXIT:
      // printf("called sys_exit\n");
      if(!check_user_mem(f->esp+4, 4, 0)) EXIT;
      f->eax = *(int*)(f->esp + 4); //return status
      thread_current()->exitval = *(int*)(f->esp + 4);

      // all child's sema up & mine down
      struct list_elem *e;
      while( list_begin( &(thread_current()->children) ) != list_end( &(thread_current()->children) ) ){
        struct thread *t = list_begin( &(thread_current()->children) );
        sema_up(&(t->waitsema));
        sema_down(&(thread_current()->waitsema));
      }

      // my sema down
      sema_down(&(thread_current()->waitsema));

      // parent의 children에서 본인 제거.
      list_remove(&(thread_current()->am_child));

      // parent's sema up
      printf ("%s: exit(%d)\n", thread_name(), f->eax);
      sema_up(&(thread_current()->parent->waitsema));
      break;


    case SYS_CREATE:
      // printf("called sys_create\n");

      if(!check_user_mem(f->esp+4, 4, 0)) EXIT;
      const char* name = *(int*)(f->esp + 4);
      // make it check buffer as well.

      if( ! check_user_mem(name, 16, 1) ) EXIT;
      
      if(!check_user_mem(f->esp+8, 4, 0)) EXIT;
      int32_t initial_size = *(int32_t*)(f->esp + 8);
      
      f->eax = filesys_create (name, initial_size);
      return;

    case SYS_OPEN:;
      // printf("called sys_open\n");
      if(!check_user_mem(f->esp+4, 4, 0)) EXIT;
      const char *file = *(int*)(f->esp + 4);
      // same as buffer in create

      if( ! check_user_mem(file, 16, 1) ) EXIT;

      // if(!check_user_mem(file)) EXIT;

      struct file* fl = filesys_open (file);
      if(fl == NULL) {
        f->eax = -1;
        return;
      }

      int openfd=2;
      while(openfd < 128){
        if( (thread_current()->fds)[openfd] == NULL ){
          (thread_current()->fds)[openfd] = fl;
          break;
        }
        openfd++;
      }
      
      f->eax = openfd;
      return;

    case SYS_WRITE: ;
      // printf("called sys_write\n");
      if(!check_user_mem(f->esp+4, 4, 0)) EXIT;
      int writefd = *(int*)(f->esp + 4);

      if(!check_user_mem(f->esp+8, 4, 0)) EXIT;
      void* writebuffer = *(int*)(f->esp + 8);
      // printf("buffer : %x\n", buffer);
      unsigned writesize;
      // printf("size : %d\n", writesize);

      if(writefd == 1){
        // same as buffer in create
  
        // if(!check_user_mem(buffer)) EXIT;
  
        if(!check_user_mem(f->esp+12, 4, 0)) EXIT;
        writesize = *(unsigned*)(f->esp + 12);
  
        if( ! check_user_mem(writebuffer, writesize, 0) ) EXIT;
        putbuf(writebuffer, writesize);
        f->eax = writesize;
        return;
      }
      else if (writefd > 0) {
        // check if writing unavilable
        if (writefd < 1 || writefd > 127 || (thread_current()->fds)[writefd] == NULL) {
          f->eax = 0;
          return;
        }  

        // buffer write할 크기만큼만 검증해야 함 = writesize = min(writesize, eof - 현 위치)
        int until_eof = file_length((thread_current()->fds)[writefd]) - (int)file_tell((thread_current()->fds)[writefd]);
        if(!check_user_mem(f->esp+12, 4, 0)) EXIT;
        writesize = *(unsigned*)(f->esp + 12);
        if (writesize > until_eof)
          writesize = until_eof;

        if( ! check_user_mem(writebuffer, writesize, 0) ) EXIT;
        
        // can write less or equal to writesize
        f->eax = file_write((thread_current()->fds)[writefd], writebuffer, writesize);
        return;
      }
      return;

    case SYS_CLOSE: ;
      // printf("called sys_close\n");
      if(!check_user_mem(f->esp+4, 4, 0)) EXIT;
      int closefd = *(int*)(f->esp + 4);
      
      if (closefd > 1 && closefd < 128 && (thread_current()->fds)[closefd] != NULL) {
        file_close((thread_current()->fds)[closefd]);
        (thread_current()->fds)[closefd] = NULL;
      }
      else
        EXIT;
      return;

    // case SYS_EXEC:
    //   if(!check_user_mem(f->esp+4, 4, 0)) EXIT;
    //   const char *cmd_line = *(int*)(f->esp + 4);
    //   if( ! check_user_mem(name, 16, 1) ) EXIT;
    //   return;


    case SYS_WAIT:
      if(!check_user_mem(f->esp+4, 4, 0)) EXIT;
      int waitfd = *(int*)(f->esp + 4);
      process_wait(waitfd);
      return;


    case SYS_REMOVE:
      if(!check_user_mem(f->esp+4, 4, 0)) EXIT;
      const char* remove_file = *(int*)(f->esp + 4);
      if( ! check_user_mem(remove_file, 16, 1) ) EXIT;
      f->eax = filesys_remove (remove_file);
      return;

    case SYS_FILESIZE:
      if(!check_user_mem(f->esp+4, 4, 0)) EXIT;
      int filesize_arg1 = *(int*)(f->esp + 4);
      struct file* filesize_file = (thread_current()->fds)[filesize_arg1];
      f->eax = file_length(filesize_file);
      return;

    case SYS_READ:
      if(!check_user_mem(f->esp+4, 4, 0)) EXIT;
      int read_fd = *(int*)(f->esp + 4);
      if( read_fd < 0 || read_fd == 1 || read_fd > 127) EXIT;

      if(!check_user_mem(f->esp+8, 4, 0)) EXIT;
      void* read_buffer = *(int*)(f->esp + 8);
      

      if(!check_user_mem(f->esp+12, 4, 0)) EXIT;
      unsigned int read_size = *(unsigned int*)(f->esp + 12);

      if( ! check_user_mem(read_buffer, read_size, 0) ) EXIT;

      struct file* read_file = (thread_current()->fds)[read_fd];
      if( read_file == NULL ) EXIT;

      f->eax = file_read (read_file, read_buffer, read_size);

		  return;


    case SYS_SEEK:
      if(!check_user_mem(f->esp+4, 4, 0)) EXIT;
      int seekfd = *(int*)(f->esp + 4);
      if(!check_user_mem(f->esp+8, 4, 0)) EXIT;
      int32_t seekpos = *(int*)(f->esp + 8);
      file_seek ((thread_current()->fds)[seekfd], seekpos);
      return;


    case SYS_TELL:
      if(!check_user_mem(f->esp+4, 4, 0)) EXIT;
      int tellfd = *(int*)(f->esp + 4);
      f->eax = (int)file_tell((thread_current()->fds)[tellfd]); 
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
    EXIT;
    shutdown_power_off();
    return;
  } 



  thread_exit ();
}
