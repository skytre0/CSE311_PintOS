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

bool check_user_mem(uint32_t *pd, int start, int end) {
  if (start == NULL)
    return false;
  if (!is_user_vaddr(end))
    return false;
  int i;
  for (i = start; i <= end; i++) {
    if (pagedir_get_page(pd, start) == NULL)
      return false;
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
  uint32_t * pd = thread_current()->pagedir;
  int number;
  if(!check_user_mem(pd,f->esp,f->esp)){
   EXIT;
  }else{
   number = *(int*)(f->esp);
  }
  
  

  switch (number)
  {
    case SYS_HALT:
      // printf("called sys_halt\n");
      shutdown_power_off();
      return;

    case SYS_EXIT:
      // printf("called sys_exit\n");
      if(!check_user_mem(pd,f->esp+4,f->esp+4)) EXIT;
      f->eax = *(int*)(f->esp + 4); //return status
      thread_current()->exitval = *(int*)(f->esp + 4);
      // my sema down
      sema_down(&(thread_current()->waitsema));

      // parent의 children에서 본인 제거.
      list_remove(&(thread_current()->am_child));

      //exitval

      
      // parent's sema up
      sema_up(&(thread_current()->parent->waitsema));
      printf ("%s: exit(%d)\n", thread_name(), f->eax);
      break;

    case SYS_CREATE:
      // printf("called sys_create\n");
      ;
      if(!check_user_mem(pd,f->esp+4,f->esp+4)) EXIT;
      const char* name = *(int*)(f->esp + 4);
      int32_t initial_size = *(int32_t*)(f->esp + 8);
      if(!check_user_mem(pd,name,name)) EXIT;
      f->eax = filesys_create (name, initial_size);
      return;

    case SYS_OPEN:;
      // printf("called sys_open\n");
      if(!check_user_mem(pd,f->esp+4,f->esp+4)) EXIT;
      const char *file = *(int*)(f->esp + 4);
      if(!check_user_mem(pd,file,file)) EXIT;

      struct file* fl = filesys_open (file);
      int openfd=2;
      while(openfd<=128){
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
      if(!check_user_mem(pd,f->esp+4,f->esp+4)) EXIT;
      int fd = *(int*)(f->esp + 4);

      if(!check_user_mem(pd,f->esp+8,f->esp+8)) EXIT;
      void* buffer = *(int*)(f->esp + 8);
      if(!check_user_mem(pd,buffer,buffer)) EXIT;

      if(!check_user_mem(pd,f->esp+12,f->esp+12)) EXIT;
      unsigned size = *(unsigned*)(f->esp + 12);
      // printf("buffer : %x\n", buffer);
      // printf("size : %d\n", size);
      if(fd == 1){
        putbuf(buffer, size);
        return size;
      }
      return;

    case SYS_CLOSE: ;
      // printf("called sys_close\n");
      if(!check_user_mem(pd,f->esp+4,f->esp+4)) EXIT;
      int closefd = *(int*)(f->esp + 4);
      
      if (closefd > 1 && closefd < 128) {
        file_close((thread_current()->fds)[closefd]);
        (thread_current()->fds)[closefd] = NULL;
      }
      return;

    // case SYS_EXEC:
    //     break;
    // case SYS_WAIT:
    //     break;
    // case SYS_REMOVE:
    //     break;
    // case SYS_FILESIZE:
    //     break;
		// case SYS_READ:
		//     break;
    // case SYS_SEEK:
    //     break;
    // case SYS_TELL:
    //     break;

    
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
    shutdown_power_off();
    break;
  } 



  thread_exit ();
}
