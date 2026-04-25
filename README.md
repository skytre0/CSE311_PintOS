# PintOS CSE311

UNIST CSE311 Operating Systems 수업에서 진행한 PintOS Project 3 구현입니다.

Contributors: [foxisdog](https://github.com/foxisdog), [skytre0](https://github.com/skytre0)

## Branches

| Branch | Project | Scope |
| --- | --- | --- |
| `main` | Project 1 | Threads |
| `project2-1` | Project 2-1 | Argument passing, basic system calls |
| `project2-2` | Project 2-2 | Extended system calls and file descriptor handling |
| `project3` | Project 3 | Virtual memory |
| `project4` | Project 4 | File system |

## Project 1: Threads

Project 1 replaces busy waiting in `timer_sleep()` with a sleep list and keeps the ready list ordered by priority.

## Project 2: User Programs

Project 2 implements argument passing, user memory validation, parent-child process synchronization, file descriptors, and file-related system calls.

The design uses process-local file descriptor lists, semaphore-based process synchronization, and explicit user pointer validation before kernel memory access.

## Project 3: Virtual Memory

Project 3 adds a supplemental page table, frame table, swap system, stack growth, and memory-mapped files. The supplemental page table is implemented as a hash table so a process can quickly find metadata for a faulting user page. Frame allocation is protected by a frame lock.

When no free frame is available, the implementation uses a clock algorithm for eviction. Accessed and dirty bits are checked through the page table, and pinned frames are excluded from eviction so that pages involved in file I/O or system calls are not removed while they are still being used.

Swap is managed with a block device, bitmap, and lock. Memory-mapped files share part of the lazy loading and eviction path with file-backed pages, but dirty mapped pages are written back to the original file instead of swap.

The design document explains that the VM synchronization uses separate locks for frame, swap, and file operations to keep more parallelism than a single global VM lock while avoiding cyclic lock dependencies.

## Design Documents

- `project1_design_document.txt`
- `project2-1_design_document.txt`
- `project2-2_design_document.txt`
- `project3_design_document.txt`
