#include "memory.h"

void *snp::Memory::_new(size_t size) 
{
  char *ptr = (char*)snp::Memory::malloc(size);
  if (size > 0 && ptr == NULL)
  {
    throw std::bad_alloc();
  }
  return ptr;
}

void snp::Memory::_delete(void * p) 
{
  snp::Memory::free(p);
} 


void* operator new(size_t size)
{
  return snp::Memory::_new(size);
}

void operator delete(void *address ) noexcept
{
  snp::Memory::_delete(address);
}

void* operator new[] ( size_t size )
{
  return snp::Memory::_new(size);
}

void operator delete[] ( void* address ) noexcept
{
  snp::Memory::_delete(address);
}
