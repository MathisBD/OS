#include "stdlib.h"


div_t div(int numer, int denom)
{
    div_t d;
    d.quot = numer / denom;
    d.rem = numer % denom; 
    return d;
}
