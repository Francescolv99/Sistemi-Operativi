#include <assert.h>
#include "m.h"

// ritorna il numero di byte per immagazzinare i bit booleani
int Bitmap_getBytes(int bits){
  return (bits%8) + (bits/8) !=0;
}

//inizializza la bitmap
void Bitmap_init(Bitmap* m int bits, uint8_t* buffer){
  m->buffer=buffer;
  m->bits=bits;
  m->dim_buffer=Bitmap_getBytes(num_bits);
}

// imposta al bit b status = 0 opppure 1
void Bitmap_setBit(Bitmap* m, int b, int stato){
  //divido per 2^3 = 8, ovvero conto il numero di byte
  int byte=b>>3;
  //verifico che il numero di byte sia conforme con la memoria
  assert(byte<m->dim_buffer); 
  //bit_a_byte assume il valore di byte se i primi due bit di "byte" sono a 1  
  //altrimenti diventa 0 
  int bit_a_byte=byte&0x03;
  //setta il bit a 0 o a 1 
  if (stato) {
    m->buffer[byte] |= (1<<bit_a_byte);
  } else {
    m->buffer[byte] &= ~(1<<bit_a_byte);
  }
}

// ritorna lo stato del bit b
int Bitmap_status(const Bitmap* m, int b){
  int byte=b>>3; 
  assert(byte<m->dim_buffer);
  int bit_a_byte=byte&0x03;
  return (m->buffer[byte] & (1<<bit_in_byte))!=0;
}
