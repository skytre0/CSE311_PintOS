#ifndef SWAP_H
#define SWAP_H

#include "vm/swap.h"
#include "devices/block.h"
#include "lib/kernel/bitmap.h"
#include "threads/vaddr.h"
#include "threads/synch.h"

static struct block *swap_block;
static struct bitmap *swap_bitmap;
static struct lock swap_lock; // 세마로할까?

void swap_init(void);

void swap_in(void *kaddr , size_t swap_index);
size_t swap_out(void *kaddr);

#endif