#pragma once
//cattura il risultato di una richiesta di allocazione
typedef enum {
  Success=0x0,
  NotEnoughMemory=-1,
  UnalignedFree=-2,  //blocco non allineato con un boundary
  OutOfRange=-3,  //blocco fuori dallo slab allocator
  DoubleFree=-4  //provo a restituire due volte lo stesso blocco
} SlabAllocatorResult;

typedef struct SlabAllocator{
  
  char* buffer;        //contiguous buffer managed by the system
  int*  free_list;     //list of linked objects
  int buffer_size;     //size of the buffer in bytes

  int size;            //number of free blocks
  int size_max;        //maximum number of blocks
  int dim_blocco;       //size of a block
  
  int first_idx;       //pointer to the first bucket
  int bucket_size;     // size of a bucket
} SlabAllocator;

SlabAllocatorResult SlabAllocator_init(SlabAllocator* allocator,
			int dim_blocco,
			int num_blocchi,
			char* memoria_blocco,
			int dim_memoria);

void* SlabAllocator_getBlock(SlabAllocator* allocator);

SlabAllocatorResult SlabAllocator_releaseBlock(SlabAllocator* allocator, void* blocco);
			
const char* SlabAllocator_strerror(SlabAllocatorResult result);
