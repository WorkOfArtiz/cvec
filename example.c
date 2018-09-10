#include <stdio.h>
#include "vector.h"

/* 
 * Creating an int vector called ivec  
 *
 * declares struct, prototypes and implementation as static functions 
 */
VEC_DECLARE_LOCAL(ivec, int)


/* 
 * Creating a const string vector called svec
 *
 * declare just struct and prototypes 
 */
VEC_DECLARE_HEADER(svec, const char *)

/* declare implementation afterwards, allowing for making into object code */
VEC_DECLARE_IMPL(svec, const char *)

int main(void)
{
    /* static allocation has to be initialised and destroyed afterwards*/
    struct ivec v;

    ivec_init(&v);
    ivec_push(&v, 1);
    ivec_push(&v, 2);
    ivec_push(&v, 3);
    ivec_push(&v, 4);

    printf("Vector[size: %ld, cap: %ld, elements: %p]\n", v.size, v.capacity, v.elements);

    /* VFOR creates a foreach, the varname has to be declared though */
    VFOR(i, &v) 
    {
        /* imagine a: int *i = next(v) */
        printf("Element : %d\n", *i);
    }

    /* free internal mem */
    ivec_destroy(&v);

    /* allocate the new string vector on the heap and init */
    svec_t *v2 = svec_create();
    svec_push(v2, "hi");
    svec_push(v2, "there");

    VFOR(s, v2) 
    {
        printf("String: '%s'\n", *s);
    }

    /* free afterwards, calls destroy */
    svec_free(v2);


    ivec_t *iv = ivec_create();

    for (int i = 0; i < 30000; i++)
        ivec_push(iv, i);

    VFOR(i, iv)
    {
        printf("%d ", *i);
    }
    puts("");

    return 0;
}