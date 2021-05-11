#include <vect.h>


void vect_delete(vect_t* vect)
{
    FREE(vect->buf);
    FREE(vect);
}