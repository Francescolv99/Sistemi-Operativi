#include <stdio.h>
#include "slab_allocator.h"

// object size=4K
# define dim_blocco 4096

// 1024 blocks
#define num_blocchi 16

// buffer should contain also bookkeeping information
#define buffer_size num_blocchi*(dim_blocco+sizeof(int))

// we allocate buffer in .bss
char buffer[buffer_size];

SlabAllocator allocator;

int main(int argc, char** argv) {
  printf("initializing... ");
  SlabAllocatorResult init_result=SlabAllocator_init(&allocator,
						     dim_blocco,
						     num_blocchi,
						     buffer,
						     buffer_size);
  printf("%s\n",SlabAllocator_strerror(init_result));

  // we allocate_all memory, and a bit more
  
  void* blocks[num_blocchi+10];
  for (int i=0; i<num_blocchi+10; ++i){
    void* block=SlabAllocator_getBlock(&allocator);
    blocks[i]=block;
    printf("allocation %d, block %p, size%d\n", i, block, allocator.size);  
  }

  // we release all memory
  for (int i=0; i<num_blocchi+10; ++i){
    void* block=blocks[i];
    if (block){
      printf("releasing... idx: %d, block %p, free %d ... ",
	     i, block, allocator.size);
      SlabAllocatorResult release_result=SlabAllocator_releaseBlock(&allocator, block);
      printf("%s\n", SlabAllocator_strerror(release_result));
    }
  }

  // we release all memory again (should get a bunch of errors)
  for (int i=0; i<num_blocchi+10; ++i){
    void* block=blocks[i];
    if (block){
      printf("releasing... idx: %d, block %p, free %d ... ",
	     i, block, allocator.size);
      SlabAllocatorResult release_result=SlabAllocator_releaseBlock(&allocator, block);
      printf("%s\n", SlabAllocator_strerror(release_result));
    }
  }
  
  // we allocate half of the memory, and release it in reverse order
  for (int i=0; i<num_blocchi-5; ++i){
    void* block=SlabAllocator_getBlock(&allocator);
    blocks[i]=block;
    printf("allocation %d, block %p, size%d\n", i, block, allocator.size);  
  }

  for (int i=num_blocchi-1; i>=0; --i){
    void* block=blocks[i];
    if (block){
      printf("releasing... idx: %d, block %p, free %d ... ",
	     i, block, allocator.size);
      SlabAllocatorResult release_result=SlabAllocator_releaseBlock(&allocator, block);
      printf("%s\n", SlabAllocator_strerror(release_result));
    }
  }

  // we allocate all  memory,
  // and release only even blocks, in reverse order
  // release odd blocks in reverse order
  for (int i=0; i<num_blocchi; ++i){
    void* block=SlabAllocator_getBlock(&allocator);
    blocks[i]=block;
    printf("allocation %d, block %p, size%d\n", i, block, allocator.size);  
  }

  for (int i=num_blocchi-1; i>=0; i-=2){
    void* block=blocks[i];
    if (block){
      printf("releasing... idx: %d, block %p, free %d ... ",
	     i, block, allocator.size);
      SlabAllocatorResult release_result=SlabAllocator_releaseBlock(&allocator, block);
      printf("%s\n", SlabAllocator_strerror(release_result));
    }
  }

  for (int i=num_blocchi-2; i>=0; i-=2){
    void* block=blocks[i];
    if (block){
      printf("releasing... idx: %d, block %p, free %d ... ",
	     i, block, allocator.size);
      SlabAllocatorResult release_result=SlabAllocator_releaseBlock(&allocator, block);
      printf("%s\n", SlabAllocator_strerror(release_result));
    }
  }


  
}
