#pragma once 

typedef struct {
    int quot;
    int rem;
} div_t;

div_t div(int number, int denom);

