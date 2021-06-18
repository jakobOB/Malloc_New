#ifndef SNP_MEMORY_H_
#define SNP_MEMORY_H_

#include "unistd.h"
#include "stdlib.h"
#include <new>
#include <mutex>

#define ALLOCATE (size_t)3*sysconf(_SC_PAGESIZE)

namespace snp {
  class Memory
  {
  public:
    struct Block_t {
      size_t size;
      bool is_used;
      struct Block_t *prev;
      struct Block_t *next;
    };

    static void reduce_heap();
    static void add_new_block(Block_t *block);
    static void merge_blocks();
    static Block_t* calculate_address(Block_t *block, size_t size);

    static void *malloc(size_t size);
    static void free(void *ptr);

    static void *_new(size_t size);
    static void _delete(void *ptr);
  };
}

void* operator new(size_t size);

void operator delete(void *address ) noexcept;

void* operator new[] ( size_t size );

void operator delete[] ( void* address ) noexcept;

#endif /* SNP_MEMORY_H_ */
