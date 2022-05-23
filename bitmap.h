#pragma once
#include <stdint.h>

typedef struct {
  uint8_t *buffer;  // externally allocated buffer
  int bits; 
  int dim_buffer;
} Bitmap;

// ritorna il numero di byte per immagazzinare i bit booleani
int Bitmap_getBit(int bits);

//inizializza la bitmap
void Bitmap_init(Bitmap* m, int bits, uint8_t* buffer);

// imposta al bit b status = 0 opppure 1
void Bitmap_setBit(Bitmap* m, int b, int stato);

// ritorna lo stato del bit b
int Bitmap_status(const Bitmap* m, int b);
