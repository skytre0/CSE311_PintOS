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
//44 page 에 인자 꺼내는 법 써져 있음
// system call 구현

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");
  printf ("vec_no : %d", f->vec_no);
//   int argc = *(int*)(f->esp+1*4);
  int* argv = *(int*)(f->esp+3*4);


  switch (f->vec_no)
  {
    case SYS_HALT:
		shutdown_power_off();
        break;
    case SYS_EXIT:
		f->eax = argv[0]; //return status
        break;
    // case SYS_EXEC:
    //     break;
    // case SYS_WAIT:
    //     break;
    case SYS_CREATE:
        break;
    // case SYS_REMOVE:
    //     break;
    case SYS_OPEN:
        break;
    // case SYS_FILESIZE:
    //     break;
		// case SYS_READ:
		//     break;
    case SYS_WRITE:
	;
		int fd = argv[0];
		const void* buffer = argv[1];
		unsigned size = argv[2];
		if(fd == 1){
			putbuf(buffer, size);
			return size;
		}
        break;
    // case SYS_SEEK:
    //     break;
    // case SYS_TELL:
    //     break;
    case SYS_CLOSE:
        break;
    
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
    break;
  
  default:
    break;
  }



  thread_exit ();
}
