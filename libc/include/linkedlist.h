#pragma once
#include <stddef.h>


// See https://doc.lagout.org/operating%20system%20/linux/Understanding%20Linux%20Kernel.pdf
// chapter 4 (processes) p.87 "Doubly Linked Lists".
// The first element in the list (the list head) is a dummy list_part_t :
// when the list is empty it points to itself, otherwise
// it points to the first and last elements in the list.
// A list_part_t is not a whole element, but is instead
// embedded in an "entry" : a struct that has a member of 
// type list_part_t (except for the list head which can be standalone).

typedef struct __ll_part {
    struct __ll_part* next;
    struct __ll_part* prev;
} ll_part_t;


// initializes the list part pointed to by head
// so that it represents an empty list
#define ll_init(head) { \
(head)->next = (head); \
(head)->prev = (head); \
}

// evaluates to true if the list is empty
#define ll_empty(head) ((head)->next == (head))

// p : pointer to a list part
// removes p from its list
// p can't be the list head
#define ll_rem(p) { \
(p)->prev->next = (p)->next; \
(p)->next->prev = (p)->prev; \
}

// n,p : pointers to list parts
// adds n after p
#define ll_add(n, p) { \
(n)->next = (p)->next; \
(p)->next->prev = (n); \
(p)->next = (n); \
(n)->prev = (p); \
}

// p : pointer to a list part 
// t : type of each entry in the list
// m : field the list_part_t corresponds to in t
// returns a pointer to the entry corresponding to p
#define ll_entry(p, t, m) \
((t*)( \
    ((void*)(p)) - \
    offsetof(t, m) \
))

// head : pointer to the list head
// p (iteration variable) : variable name (will be a pointer to a list part)
#define ll_for_each(p, head) \
for (ll_part_t* p = (head)->next; p != (head); p = p->next)

// head : pointer to the list head
// e (iteration variable) : pointer to a list entry
// t : type of each entry in the list
// m : field the list_part_t corresponds to in t
#define ll_for_each_entry(e, head, t, m) \
for (t* e = ll_entry((head)->next, t, m); \
    &(e->m) != (head); \
    e = ll_entry(e->m.next, t, m)) 

