# PintOS CSE311

UNIST CSE311 Operating Systems 수업에서 진행한 PintOS Project 2-1 구현입니다.

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

Project 1 replaces busy waiting in `timer_sleep()` with a sleep list. The implementation keeps the PintOS thread state model mostly unchanged by adding sleep metadata to each thread and waking blocked threads from the timer interrupt.

The ready list is kept ordered by priority so the scheduler can choose the highest-priority ready thread directly.

## Project 2-1: Argument Passing and Basic System Calls

Argument passing is implemented by parsing the command line with `strtok_r()` and pushing arguments onto the user stack in the order expected by PintOS. The implementation explicitly checks stack bounds so that argument setup does not overflow the stack page.

The system call work introduces per-process file descriptor state and parent-child process bookkeeping. Parent and child processes synchronize with semaphores so that `wait()` can receive the child exit status correctly. The design uses process-local file descriptor lists because file descriptors only need to be unique within a process.

## Design Documents

- `project1_design_document.txt`
- `project2-1_design_document.txt`
