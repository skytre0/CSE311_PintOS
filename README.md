# PintOS CSE311

UNIST CSE311 Operating Systems 수업에서 진행한 PintOS 프로젝트입니다.

Contributors: [foxisdog](https://github.com/foxisdog), [skytre0](https://github.com/skytre0)

## Branches

| Branch | Project | Scope |
| --- | --- | --- |
| `main` | Project 1 | Threads |
| `project2-1` | Project 2-1 | Argument passing, basic system calls |
| `project2-2` | Project 2-2 | Extended system calls and file descriptor handling |
| `project3` | Project 3 | Virtual memory |
| `project4` | Project 4 | File system |

`project4` is the final branch and contains the cumulative implementation from the previous projects.

## Project 1: Threads

Project 1 replaces busy waiting in `timer_sleep()` with a sleep list. Each blocked thread stores the tick at which it should wake up, and the timer interrupt checks the sleep list to unblock threads whose wake time has passed.

For scheduling, the ready list is kept ordered by priority. This makes scheduling simple: inserting a ready thread preserves priority order, and the scheduler can choose the front of the list. The design document explains that this approach was chosen to keep the implementation close to the PintOS template and reduce unnecessary changes to the thread state model.

## Project 2-1: Argument Passing and Basic System Calls

Argument passing is implemented by parsing the command line with `strtok_r()` and pushing arguments onto the user stack in the order expected by PintOS. The implementation explicitly checks stack bounds so that argument setup does not overflow the stack page.

The system call work introduces per-process file descriptor state and parent-child process bookkeeping. Parent and child processes synchronize with semaphores so that `wait()` can receive the child exit status correctly. The design uses process-local file descriptor lists because file descriptors only need to be unique within a process.

## Project 2-2: Extended System Calls

Project 2-2 extends system call behavior for file operations such as `read`, `write`, `exec`, `wait`, `remove`, `filesize`, `seek`, and `tell`.

User memory is validated before kernel access by checking null pointers, `PHYS_BASE`, and page table mappings. Invalid user memory terminates the process through the exit path so temporary process resources are released consistently. File system operations are protected with a semaphore, and the currently executing file is tracked to prevent writes to an executable that is running.

## Project 3: Virtual Memory

Project 3 adds a supplemental page table, frame table, swap system, stack growth, and memory-mapped files. The supplemental page table is implemented as a hash table so a process can quickly find metadata for a faulting user page. Frame allocation is protected by a frame lock.

When no free frame is available, the implementation uses a clock algorithm for eviction. Accessed and dirty bits are checked through the page table, and pinned frames are excluded from eviction so that pages involved in file I/O or system calls are not removed while they are still being used.

Swap is managed with a block device, bitmap, and lock. Memory-mapped files share part of the lazy loading and eviction path with file-backed pages, but dirty mapped pages are written back to the original file instead of swap.

## Project 4: File System

Project 4 changes PintOS file allocation from a single contiguous extent to an indexed inode structure. The inode stores direct block pointers, one indirect block pointer, and one doubly indirect block pointer. This was chosen because direct pointers keep small file access simple and fast, while indirect and doubly indirect blocks allow files to grow beyond the original contiguous allocation limit.

The inode layout supports a maximum file size of about 8.1 MB. File growth and file system operations are synchronized so that concurrent reads, writes, and file extension do not corrupt inode metadata or expose uninitialized sectors as valid file data.

## Design Documents

- `project1_design_document.txt`
- `project2-1_design_document.txt`
- `project2-2_design_document.txt`
- `project3_design_document.txt`
- `project4_design_document.txt`
