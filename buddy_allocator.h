#pragma once
#include "slab_allocator.h"
#include "linked_list.h"

#define MAX_LEVELS 16

// una entry della buddy list
typedef struct BuddyListItem { //derivare da una bitmap e non da una lista
  /* bitmap */
  ListItem list;
  /* bitmap */
  int idx_albero;   // indice albero
  int level; // livello del buddy (nell'albero)
  char* start; // inizio della memoria
  int size;
  /* bitmap */
  struct BuddyListItem* buddy_ptr;
  struct BuddyListItem* parent_ptr;
  /* bitmap */
} BuddyListItem;


typedef struct  {
  /* bitmap */
  ListHead free[MAX_LEVELS];
  /* bitmap */
  int livelli;
  /* bitmap */
  SlabAllocator slab_allocator;
  /* bitmap */
  char* memoria; 
  int min_bucket_size; // la minima RAM che può essere ritornata
} BuddyAllocator;


int BuddyAllocator_calcola_dim(int livelli);

void BuddyAllocator_init(BuddyAllocator* buddy,
                         int livelli,
                         char* buffer,
                         int dim_buffer,
                         char* memoria,
                         int min_bucket_size);

BuddyListItem* BuddyAllocator_getBuddy(BuddyAllocator* buddy, int level);

void BuddyAllocator_releaseBuddy(BuddyAllocator* buddy, BuddyListItem* item);

void* BuddyAllocator_malloc(BuddyAllocator* buddy, int size);

void BuddyAllocator_free(BuddyAllocator* buddy, void* mem);
