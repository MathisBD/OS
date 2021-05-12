#pragma once
#include <stdint.h>


// a resizable array that contains void pointers
typedef struct {
    void** buf;
    uint32_t size;   
    uint32_t capacity;
} vect_t;


vect_t* vect_create();
void vect_delete(vect_t* vect);
void vect_grow(vect_t* vect, uint32_t cap);
void* vect_get(vect_t* vect, uint32_t i);
// returns the previous element stored at i.
void* vect_set(vect_t* vect, uint32_t i, void* val);
void vect_append(vect_t* vect, void* val);
void* vect_pop(vect_t* vect);
// returns a pointer to the underlying array
void** vect_get_array(vect_t* vect);

