# PintOS CSE311

UNIST CSE311 Operating Systems 수업에서 진행한 PintOS Project 1 구현입니다.

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

Project 1 replaces busy waiting in `timer_sleep()` with a sleep list. Each blocked thread stores the tick at which it should wake up, and the timer interrupt checks the sleep list to unblock threads whose wake time has passed.

For scheduling, the ready list is kept ordered by priority. This makes scheduling simple: inserting a ready thread preserves priority order, and the scheduler can choose the front of the list. The design document explains that this approach was chosen to keep the implementation close to the PintOS template and reduce unnecessary changes to the thread state model.

## Design Document

- `project1_design_document.txt`
