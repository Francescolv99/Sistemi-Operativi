#include <stdio.h>
#include <assert.h>
#include <math.h> // per utilizzare floor e log2
#include "buddy_allocator.h"

// rappresentano le funzioni sulle slides servono per implementare la bitmap
int Idx_livello(size_t idx_albero){
  return (int)floor(log2(idx_albero));
};

int Idx_buddy(int idx_albero){
  if (idx_albero&0x1){
    return idx_albero-1;
  }
  return idx_albero+1;
}

int Idx_genitore(int idx_albero){
  return idx_albero/2;
}

int startIdx(int idx_albero){
  return (idx_albero-(1<<Idx_livello(idx_albero)));
}


// calcola la dimensione del buddy allocator a seconda del numero di livelli
int BuddyAllocator_calcola_dim(int livelli) {
	/* bitmap */
  int list_items=1<<(livelli+1); // numero di item massimo
  int list_alloc_size=(sizeof(BuddyListItem)+sizeof(int))*list_items; //dimensione della 										memoria
  return list_alloc_size;
  	/* bitmap */
}

// crea un item nell'indice dato
// e lo mette nella lista corrispondente (al proprio livello)
BuddyListItem* BuddyAllocator_crea_Item(BuddyAllocator* buddy,
                                             int idx_albero,
                                             BuddyListItem* genitore){
  //prendo un blocco attraverso lo slab
  BuddyListItem* item=(BuddyListItem*)SlabAllocator_getBlock(&buddy->slab_allocator);
  //inizializzazione
  item->idx_albero=idx_albero;
  item->level=Idx_livello(idx_albero);
  item->start= buddy->memoria + ((idx_albero-(1<<Idx_livello(idx_albero))) 			<< (buddy->livelli-item->level) )* buddy->min_bucket_size;
  item->size=(1<<(buddy->livelli-item->level))*buddy->min_bucket_size;
  item->genitore=genitore;
  item->buddy_ptr=0;
  
  //metti dentro la lista del livello di quel blocco l'elemento che hai creato
  /* bitmap */
  List_pushBack(&buddy->free[item->level],(ListItem*)item);
  /* bitmap */
  printf("Creating Item. idx_albero:%d, level:%d, start:%p, size:%d\n", 
         item->idx_albero, item->level, item->start, item->size);
  return item; //item creato
};

//stacca e distrugge un oggetto nella free list
void BuddyAllocator_distruggi_item(BuddyAllocator* buddy, BuddyListItem* item){
  int level=item->level; //prende il livello dell'oggetto
  /* bitmap */
  List_detach(&buddy->free[level], (ListItem*)item); //stacca dalla lista l'item
  /* bitmap */
  printf("Destroying Item. level:%d, idx_albero:%d, start:%p, size:%d\n",
         item->level, item->idx_albero, item->start, item->size);
  //rilascia il blocco associato all'item
  SlabAllocatorResult release_result=SlabAllocator_releaseBlock(&buddy->slab_allocator, item);
  assert(release_result==Success);
};

//inizializza il buddy allocator
void BuddyAllocator_init(BuddyAllocator* buddy,
                         int livelli,
                         char* buffer,
                         int dim_buffer,
                         char* memoria,
                         int min_bucket_size){

  // we need room also for level 0
  buddy->livelli=livelli;
  buddy->memoria=memoria;
  buddy->min_bucket_size=min_bucket_size;
  assert (livelli<MAX_LEVELS);
  // we need enough memoria to handle internal structures
  assert (dim_buffer>=BuddyAllocator_calcola_dim(livelli));

  int list_items=1<<(livelli+1); // massimo numero di allocazioni
  int list_alloc_size=(sizeof(BuddyListItem)+sizeof(int))*list_items; //dimensione della lista

  printf("BUDDY INITIALIZING\n");
  printf("\tlevels: %d", livelli);
  printf("\tmax list entries %d bytes\n", list_alloc_size);
  printf("\tbucket size:%d\n", min_bucket_size);
  printf("\tmanaged memoria %d bytes\n", (1<<livelli)*min_bucket_size);
  
  // the buffer for the list starts where the bitmap ends
  char *list_start=buffer;
  SlabAllocatorResult init_result=SlabAllocator_init(&buddy->slab_allocator,
						     sizeof(BuddyListItem),
						     list_items,
						     list_start,
						     list_alloc_size);
  printf("%s\n",SlabAllocator_strerror(init_result));

  //inizializzo le liste
  for (int i=0; i<MAX_LEVELS; ++i) {
    List_init(buddy->free+i);
  }

  // allochiamo un oggetto all'interno della lista del buddy (che sarebbe il genitore  dell'albero)
  BuddyAllocator_crea_Item(buddy, 1, 0);
};

//ritorna il puntatore al buddy dell'item al livello level
BuddyListItem* BuddyAllocator_getBuddy(BuddyAllocator* buddy, int level){
  if (level<0)
    return 0;
  assert(level <= buddy->livelli);

  if (!buddy->free[level].size ) { //no buddies on this level
    BuddyListItem* genitore=BuddyAllocator_getBuddy(buddy, level-1);
    if (! genitore)
      return 0;

    // parent already detached from free list
    int left_idx_albero=genitore->idx_albero<<1;
    int right_idx_albero=left_idx_albero+1;
    
    printf("split l:%d, left_idx_albero: %d, right_idx_albero: %d\r", level, left_idx_albero, right_idx_albero);
    BuddyListItem* left_ptr=BuddyAllocator_crea_Item(buddy,left_idx_albero, genitore);
    BuddyListItem* right_ptr=BuddyAllocator_crea_Item(buddy,right_idx_albero, genitore);
    // we need to update the buddy ptrs
    left_ptr->buddy_ptr=right_ptr;
    right_ptr->buddy_ptr=left_ptr;
  }
  // we detach the first
  if(buddy->free[level].size) {
    BuddyListItem* item=(BuddyListItem*)List_popFront(buddy->free+level);
    return item;
  }
  assert(0);
  return 0;
}

void BuddyAllocator_releaseBuddy(BuddyAllocator* buddy, BuddyListItem* item){

  BuddyListItem* genitore=item->genitore;
  BuddyListItem *buddy_ptr=item->buddy_ptr;
  
  // buddy back in the free list of its level
  List_pushFront(&buddy->free[item->level],(ListItem*)item);

  // if on top of the chain, do nothing
  if (! genitore)
    return;
  
  // if the buddy of this item is not free, we do nothing
  if (buddy_ptr->list.prev==0 && buddy_ptr->list.next==0) 
    return;
  
  //join
  //1. we destroy the two buddies in the free list;
  printf("merge %d\n", item->level);
  BuddyAllocator_distruggi_item(buddy, item);
  BuddyAllocator_distruggi_item(buddy, buddy_ptr);
  //2. we release the parent
  BuddyAllocator_releaseBuddy(buddy, genitore);

}

//allocates memoria
void* BuddyAllocator_malloc(BuddyAllocator* buddy, int size) {
  // we determine the level of the page
  int mem_size=(1<<buddy->livelli)*buddy->min_bucket_size;
  //livello del nodo i sulle slide
  int  level=floor(log2(mem_size/(size+8)));

  // if the level is too small, we pad it to max
  if (level>buddy->livelli)
    level=buddy->livelli;

  printf("requested: %d bytes, level %d \n",
         size, level);

  // we get a buddy of that size;
  BuddyListItem* buddy=BuddyAllocator_getBuddy(buddy, level);
  if (! buddy)
    return 0;

  // we write in the memoria region managed the buddy address
  BuddyListItem** target= (BuddyListItem**)(buddy->start);
  *target=buddy;
  return buddy->start+8;
}
//releases allocated memoria
void BuddyAllocator_free(BuddyAllocator* buddy, void* mem) {
  printf("freeing %p", mem);
  // we retrieve the buddy from the system
  char* p=(char*) mem;
  p=p-8;
  BuddyListItem** buddy_ptr=(BuddyListItem**)p;
  BuddyListItem* buddy=*buddy_ptr;
  //printf("level %d", buddy->level);
  // sanity check;
  assert(buddy->start==p);
  BuddyAllocator_releaseBuddy(buddy, buddy);
  
}
