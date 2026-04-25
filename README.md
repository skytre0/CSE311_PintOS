# PintOS CSE311

UNIST CSE311 Operating Systems 수업에서 진행한 PintOS Project 2-2 구현입니다.

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

## Project 2-1: Argument Passing and Basic System Calls

Arguments are parsed with `strtok_r()` and placed on the user stack according to the PintOS calling convention. Stack overflow is checked while building the initial stack.

The first system call implementation adds parent-child process tracking, semaphore-based `wait()`/`exit()` synchronization, and process-local file descriptor lists.

## Project 2-2: Extended System Calls

Project 2-2 extends system call behavior for file operations such as `read`, `write`, `exec`, `wait`, `remove`, `filesize`, `seek`, and `tell`.

User memory is validated before kernel access by checking null pointers, `PHYS_BASE`, and page table mappings. Invalid user memory terminates the process through the exit path so temporary process resources are released consistently. File system operations are protected with a semaphore, and the currently executing file is tracked to prevent writes to an executable that is running.

The design document explains that file descriptors are stored in a list instead of a fixed-size array, trading slower lookup for a flexible number of open files within memory limits.

## Design Documents

- `project1_design_document.txt`
- `project2-1_design_document.txt`
- `project2-2_design_document.txt`
