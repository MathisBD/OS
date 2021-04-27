#pragma once
#include <stdbool.h>
#ifdef __is_libk
#include "memory/kheap.h"
#endif


// doubly linked lists
// to store pointers to objects.
// lists can be manipulated directly 
// (e.g. iterating on the nodes) and/or
// through the functions listed here

typedef struct __list_node {
    void* contents;
    struct __list_node* next;
    struct __list_node* prev;
} list_node_t;

typedef struct {
    list_node_t* first;
    list_node_t* last;
} list_t;

list_t* list_create(); // allocates a new list on the heap
void list_delete(list_t* l);
bool list_empty(list_t* l);
void list_add_front(list_t* l, void* elem);
void list_add_back(list_t* l, void* elem);
void* list_pop_front(list_t* l);
void* list_pop_back(list_t* l);
