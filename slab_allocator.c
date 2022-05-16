#include "slab_allocator.h"

static const int NullIdx=-1;
static const int DetachedIdx=-2;

static const char* SlabAllocator_strerrors[]=
  {"Success",
   "NotEnoughMemory",
   "UnalignedFree",
   "OutOfRange",
   "DoubleFree",
   0
  };

const char* SlabAllocator_strerror(SlabAllocatorResult result) {
  return SlabAllocator_strerrors[-result];
}

SlabAllocatorResult SlabAllocator_init(SlabAllocator* a,int dim_blocco,int num_blocchi,
					char* memoria_blocco,int dim_memoria) {

  // we first check if we have enough memory
  // for the bookkeeping
  int dim_richiesta= num_blocchi*(dim_blocco+sizeof(int));
  if (dim_memoria<dim_richiesta)
    return NotEnoughMemory;

  a->dim_blocco=dim_blocco;
  a->size=num_blocchi;
  a->buffer_size=dim_blocco*num_blocchi;
  a->size_max = num_blocchi;
  
  //la memoria sarà divisa in dati + array list
  a->buffer=memoria_blocco; // the upper part of the buffer is used as memory
  a->free_list= (int*)(memoria_blocco+dim_blocco*num_blocchi); // the lower part is for bookkeeping

  // now we populate the free list by constructing a linked list
  for (int i=0; i<a->size-1; ++i){
    a->free_list[i]=i+1;
  }
  // set the last element to "NULL" 
  a->free_list[a->size-1] = NullIdx;
  a->first_idx=0;
  return Success;
}

//ritorna il puntatore al primo blocco disponibile
void* SlabAllocator_getBlock(SlabAllocator* a) {
  if (a->first_idx==-1)
    return 0;

  // we need to remove the first bucket from the list
  // trovo l'indice del primo blocco disponibile
  int detached_idx = a->first_idx; 
  // advance the head
  a->first_idx=a->free_list[a->first_idx];
  --a->size;
  
  a->free_list[detached_idx]=DetachedIdx;//segnalo che ho preso il blocco
  
  //now we retrieve the pointer in the item buffer
  char* ind_blocco=a->buffer+(detached_idx*a->dim_blocco);
  return ind_blocco;
}

SlabAllocatorResult SlabAllocator_releaseBlock(SlabAllocator* a, void* blocco_){
  //we need to find the index from the address
  char* blocco=(char*) blocco_;
  int offset=blocco - a->buffer;

  //sanity check, we need to be aligned to the block boundaries
  if (offset%a->dim_blocco)
    return UnalignedFree;

  int idx=offset/a->dim_blocco;

  //sanity check, are we inside the buffer?
  if (idx<0 || idx>=a->size_max)
    return OutOfRange;

  //is the block detached?
  if (a->free_list[idx]!=DetachedIdx)
    return DoubleFree;

  // all fine, we insert in the head
  a->free_list[idx]=a->first_idx;
  a->first_idx=idx;
  ++a->size;
  return Success;
}
