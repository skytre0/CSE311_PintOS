#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

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




// switch case 문으로 경우에 대해서 분기
// 44 page 에 인자 꺼내는 법 써져 있음
// system call 구현

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n\n");

  int tmp=f->esp;
  // while(tmp+4<= 0xc0000000){
  //   printf("Address: %8x    Data: %8x\n", tmp, *(int*)tmp);  
  //   tmp+=4;
  // }

  // user process의 syscall
  int number = *(int*)(f->esp);

  switch (number)
  {
    case SYS_HALT:
      printf("called sys_halt\n");
      shutdown_power_off();
      return;

    case SYS_EXIT:
      printf("called sys_exit\n");
      f->eax = *(int*)(f->esp + 4); //return status
      // my sema down
      sema_down(&(thread_current()->waitsema));

      // parent의 children에서 본인 제거.
      
      // parent's sema up
      sema_up(&(thread_current()->parent->waitsema));
      printf ("%s: exit(%d)\n", thread_name(), f->eax);
      break;

    case SYS_CREATE:
      printf("called sys_create\n");
      shutdown_power_off();
      return;

    case SYS_OPEN:
      printf("called sys_open\n");
      shutdown_power_off();
      return;

    case SYS_WRITE:
      printf("called sys_write\n");
      int fd = *(int*)(f->esp + 4);
      void* buffer = *(int*)(f->esp + 8);
      unsigned size = *(unsigned*)(f->esp + 12);
      printf("buffer : %x\n", buffer);
      printf("size : %d\n", size);
      if(fd == 1){
        putbuf(buffer, size);
        return size;
      }
      return;

    case SYS_CLOSE:
      printf("called sys_close\n");
      shutdown_power_off();
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
