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

// struct intr_frame
//   {
//     /* Pushed by intr_entry in intr-stubs.S.
//        These are the interrupted task's saved registers. */
//     uint32_t edi;               /* Saved EDI. */
//     uint32_t esi;               /* Saved ESI. */
//     uint32_t ebp;               /* Saved EBP. */
//     uint32_t esp_dummy;         /* Not used. */
//     uint32_t ebx;               /* Saved EBX. */
//     uint32_t edx;               /* Saved EDX. */
//     uint32_t ecx;               /* Saved ECX. */
//     uint32_t eax;               /* Saved EAX. */
//     uint16_t gs, :16;           /* Saved GS segment register. */
//     uint16_t fs, :16;           /* Saved FS segment register. */
//     uint16_t es, :16;           /* Saved ES segment register. */
//     uint16_t ds, :16;           /* Saved DS segment register. */


// switch case 문으로 경우에 대해서 분기
//44 page 에 인자 꺼내는 법 써져 있음
// system call 구현

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");
  thread_exit ();
}
