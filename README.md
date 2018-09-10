# cvec

A generic vector implementation in C (mis)using macros to create strongly typed containers.

Simple example:
```
#include "vector.h"

VEC_DECLARE_LOCAL(ivec, int)

int main(void)
{
  struct ivec *v = ivec_create();

  for (int i = 0; i < 20; i++)
      ivec_push(v, i);

  printf("Last element is: %d\n", ivec_top(v));

  VFOR(i, v)
      printf("element: %d\n", *i);

  return 0;
}
```

## interface of your custom vector

For this we're gonna assume you called it like `VEC_DECLARE_LOCAL(ivec, int)`

```{c}
void    ivec_init(ivec_t *v);                         initialise a stack allocated struct
void    ivec_destroy(ivec_t *v)                       destroys stack allocated struct

ivec_t *ivec_create();                                alloc and initialise a struct on heap
void    ivec_free(ivec_t *v)                          destroys and frees allocated struct from heap

ivec_t *ivec_push(ivec_t *v, int i)                   add one element
int     ivec_pop(ivec_t *v)                           pop one element
int     ivec_top(ivec_t *v)                           get the last element
ivec_t *ivec_extend(ivec_t *v, int *arr, size_t size) add size elements

You can also check it's internals directly for size and access to elements
of your struct ivec. (aliased to ivec_t)

struct ivec
{
   int    *elements;
   size_t  size;
   size_t  capacity;
};
```

## when to use what macro

I had 2 use cases, quick usage, you just need a vector somewhere in a c file:

```{c}
/* magic to declare and implement vector operations */

void method()
{
  struct integer_vector v;
  integer_vector_init(&v);

  ...
}
```

Or you want a vector in your own objects with headers etc.

header:
```{c}
/* magic to just declare vec type and interface */

struct object
{
  vec_object_pointers_t linked_to;
  vec_object_pointers_t objects_linked_to_this;
}
```

```{c}
#include header
/* magic to generate implementation */

...
```

Ideally you'd want to both be able to declare a quick static implementation
if you won't reuse it, and a another one if you want more proper usage.

For this there are 3 variants:

```
VEC_DECLARE_LOCAL, generates the struct and the methods as static.
VEC_DECLARE_HEADER, generates the struct and function prototypes.
VEC_DECLARE_IMPL, generates just the implementation.
```
