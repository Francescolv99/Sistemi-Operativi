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
  
  char* buffer;        
  int*  free_list;     //linked list
  int buffer_size;     //dimensione del buffer in bytes

  int size;            //numero di blocchi liberi
  int size_max;        //numero massimo di blocchi
  int dim_blocco;       //dimensione di un blocco
  
  int first_idx;       //puntatore al primo bucket
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
