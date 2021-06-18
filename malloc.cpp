#include "memory.h"

static snp::Memory::Block_t *root_block = NULL;
static snp::Memory::Block_t *last_block = NULL;
static std::mutex malloc_mutex;
static std::mutex free_mutex;

void *snp::Memory::malloc(size_t size){
  malloc_mutex.lock();
  if (size == 0) {
    malloc_mutex.unlock();
    return NULL;
  }
  size_t alloc_size = (size + sizeof(Block_t)) > ALLOCATE ? size + sizeof(Block_t) : ALLOCATE;
  Block_t *block, *new_block;

  block = root_block;
  while (block)
  {
    if (!block->is_used)
    {
      if (block->size > (size + 2 * sizeof(Block_t)))
      {
        new_block = calculate_address(block, size + sizeof(Block_t));
        new_block->size = block->size - size - sizeof(Block_t);
        new_block->is_used = false;
        block->size = size + sizeof(Block_t);
        block->is_used = true;
        add_new_block(new_block);
        malloc_mutex.unlock();
        return calculate_address(block, sizeof(Block_t));
      }
      else if (block->size == size + sizeof(Block_t))
      {
        block->is_used = true;
        malloc_mutex.unlock();
        return calculate_address(block, sizeof(Block_t));
      }
      else if(block->prev && !block->prev->is_used)
      {
        merge_blocks();
      }
    }
    block = block->next;
  }
  // If no block found, create new block with sbrk(alloc_size)
  // If size < ALLOCATE, create 2 blocks
  // return block with requested size
  block = (Block_t*)(sbrk(alloc_size));
  if ((int)block == -1)
  {
    malloc_mutex.unlock();
    return NULL;
  }

  if (alloc_size > (size + 2 * sizeof(Block_t)))
  {
    block->size = size + sizeof(Block_t);
    block->is_used = true;
    add_new_block(block);
    new_block = calculate_address(block, block->size);
    new_block->size = alloc_size - block->size;
    new_block->is_used = false;
    add_new_block(new_block);
  }
  else
  {
    block->size = alloc_size;
    block->is_used = true;
    add_new_block(block);
  }
  merge_blocks();
  malloc_mutex.unlock();
  return calculate_address(block, sizeof(Block_t));
}


void snp::Memory::free(void *ptr){
  free_mutex.lock();
  if (!ptr) {
    free_mutex.unlock();
    return;
  }

  Block_t *block = root_block;
  bool found = false;

  while (block)
  {
    if (calculate_address(block, sizeof(Block_t)) == ptr)
    {
      found = block->is_used;
      break;
    }
    block = block->next;
  }

  if(!found) {
    free_mutex.unlock();
    exit(-1);
  }

  block->is_used = false;
  merge_blocks();
  reduce_heap();
  free_mutex.unlock();
}

void snp::Memory::add_new_block(snp::Memory::Block_t *block) {
  block->next = NULL;
  block->prev = NULL;

  if(!root_block || root_block > block)
  {
    if (root_block)
    {
      block->next = root_block;
      root_block->prev = block;
    }
    else {
      last_block = block;
    }
    root_block = block;
  }
  else {
    if (last_block < block)
    {
      block->prev = last_block;
      last_block->next = block;
      last_block = block;
    }
    else {
      Block_t *tmp_block = root_block;
      while (tmp_block->next && tmp_block->next < block)
      {
        tmp_block = tmp_block->next;
      }
      tmp_block->next->prev = block;
      block->prev = tmp_block;
      block->next = tmp_block->next;
      tmp_block->next = block;
    }
  }
}

void snp::Memory::merge_blocks() {
  Block_t *block = root_block;

  // Merge not used blocks
  while (block->next)
  {
    Block_t *next_block = block->next;
    bool used = !block->is_used && !next_block->is_used;
    if(used && calculate_address(block, block->size) == next_block)
    {
      block->size += next_block->size;
      if(next_block->next)
      {
        block->next = next_block->next;
        next_block->next->prev = block;
      }
      else
      {
        block->next = NULL;
        last_block = block;
        break;
      }
    }
    block = block->next;
  }
}

snp::Memory::Block_t *snp::Memory::calculate_address(snp::Memory::Block_t *block, size_t size) {
  return (Block_t*)(((char*)block) + size);
}

void snp::Memory::reduce_heap() {
  Block_t *block;
  char *program_break = (char*)sbrk(0);

  if (last_block && !last_block->is_used && (char *) calculate_address(last_block, last_block->size) == program_break)
  {
    char* new_brk = (char*)last_block;
    if (last_block->prev)
    {
      block = last_block->prev;
      block->next = NULL;
      last_block = block;
    }
    else
    {
      root_block = NULL;
      last_block = NULL;
    }

    if(brk(new_brk) != 0)
    {
      free_mutex.unlock();
      exit(-1);
    }
  }
}
