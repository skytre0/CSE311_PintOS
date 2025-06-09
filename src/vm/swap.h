#ifndef SWAP_H
#define SWAP_H

#include "devices/block.h"
#include "lib/kernel/bitmap.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "frame.h"
#include "userprog/syscall.h"

static struct block *swap_block;
static struct bitmap *swap_bitmap;
static struct lock swap_lock; // 세마로할까?

void swap_init(void);

void swap_in(struct frame* new_frame, struct supplemental_page* sp);
struct frame* swap_out(void);
void rm_swap(struct supplemental_page* sp);

#endif