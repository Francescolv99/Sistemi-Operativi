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

  // controllo che ci sia abbastanza memoria
  int dim_richiesta= num_blocchi*(dim_blocco+sizeof(int));
  if (dim_memoria<dim_richiesta)
    return NotEnoughMemory;

  a->dim_blocco=dim_blocco;
  a->size=num_blocchi;
  a->buffer_size=dim_blocco*num_blocchi;
  a->size_max = num_blocchi;
  
  //la memoria sarà divisa in dati + array list
  a->buffer=memoria_blocco; 
  a->free_list= (int*)(memoria_blocco+dim_blocco*num_blocchi); 

  // popolo la free_list utilizzando una linked list
  for (int i=0; i<a->size-1; ++i){
    a->free_list[i]=i+1;
  }
  //setta l'ultimo elemento a NULL 
  a->free_list[a->size-1] = NullIdx;
  a->first_idx=0;
  return Success;
}

//ritorna il puntatore al primo blocco disponibile
void* SlabAllocator_getBlock(SlabAllocator* a) {
  if (a->first_idx==-1)
    return 0;

  // trovo l'indice del primo blocco disponibile
  int detached_idx = a->first_idx; 
  // sposta in avanti la testa
  a->first_idx=a->free_list[a->first_idx];
  --a->size;
  
  a->free_list[detached_idx]=DetachedIdx;//segnalo che ho preso il blocco
  
  //setto il puntatore al blocco
  char* ind_blocco=a->buffer+(detached_idx*a->dim_blocco);
  return ind_blocco;
}

SlabAllocatorResult SlabAllocator_releaseBlock(SlabAllocator* a, void* blocco_){
  //trovo l'indirizzo del blocco
  char* blocco=(char*) blocco_;
  int offset=blocco - a->buffer;

  //controllo che sia al limite di un boundaries
  if (offset%a->dim_blocco)
    return UnalignedFree;

  int idx=offset/a->dim_blocco;

  //controllo di essere all'interno del buffer
  if (idx<0 || idx>=a->size_max)
    return OutOfRange;

  //controllo che il blocco sia staccato dalla free_list
  if (a->free_list[idx]!=DetachedIdx)
    return DoubleFree;

  //inserisco in testa
  a->free_list[idx]=a->first_idx;
  a->first_idx=idx;
  ++a->size;
  return Success;
}
