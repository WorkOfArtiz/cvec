/* 
 * The C vector template header
 * 
 * For static usage:
 * VEC_DECLARE_LOCAL(Type)
 *  e.g. VEC_DECLARE_LOCAL(Type) => a struct vec_int/vec_int_t with vec_int_push etc.
 * 
 * VEC_DECLARE_LOCAL(VectorName, Type)
 *  e.g. VEC_DECLARE_LOCAL(ivec, int) => a struct ivec/ivec_t with ivec_push etc.
 * 
 * There's also
 * VEC_DECLARE_HEADER(Type)
 * VEC_DECLARE_HEADER(VectorName, Type)
 *  Which declares the struct and prototypes
 * 
 * VEC_DECLARE_IMPL(Type)
 * VEC_DECLARE_IMPL(VectorName, Type)
 *  Which declares only the functions (you'll need include a header with the
 *   header declaration)
 * 
 * The exposed interface is for instance for VEC_DECLARE_LOCAL(ivec, int):
 *  void    ivec_init(ivec_t *v);                         initialise a stack allocated struct
 *  void    ivec_destroy(ivec_t *v)                       destroys stack allocated struct
 *
 *  ivec_t *ivec_create();                                alloc and initialise a struct on heap
 *  void    ivec_free(ivec_t *v)                          destroys and frees allocated struct from heap
 *
 *  ivec_t *ivec_push(ivec_t *v, int i)                   add one element
 *  int     ivec_pop(ivec_t *v)                           pop one element
 *  int     ivec_top(ivec_t *v)                           get the last element
 *  ivec_t *ivec_extend(ivec_t *v, int *arr, size_t size) add size elements
 * 
 * You can also check it's internals directly for size and access to elements
 *  struct ivec 
 *  {
 *     int    *elements;
 *     size_t  size;
 *     size_t  capacity;
 *  }
 * 
 * There's also VFOR which iterates over any vector object, syntax:
 * 
 * ivec_t *v = ivec_create();
 * 
 * VFOR(Varname, v)
 * {
 *     ... body ...
 * }
 * 
 * =>
 * 
 * for (int i = 0; i < v->size; i++)
 * {
 *     int *Varname = &v->elements[i];
 * 
 *     ... body ...
 * }
 * 
 * This allows for easy modification (due to it being a pointer) and accessing 
 * elements.
 * 
 * 
 * Full Code example:
 * #include "vector.h"
 * 
 * VEC_DECLARE_LOCAL(ivec, int)
 * 
 * int main(void)
 * {
 *       struct ivec *v = ivec_create();
 *       
 *       for (int i = 0; i < 20; i++)
 *           ivec_push(v, i);
 * 
 *       printf("Last element is: %d\n", ivec_top(v));
 *       
 *       VFOR(i, v)
 *           printf("element: %d\n", *i);
 *
 *       return 0;
 * }
 */
#ifndef VECTOR_HEADER
#define VECTOR_HEADER
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

/* helpers for declaring the structs */

/* variadic identifier concatenation */
#define V__CONCAT1(ident1) ident1
#define V__CONCAT2(ident1, ident2) ident1 ## _ ## ident2
#define V__CONCAT3(ident1, ident2, ident3) ident1 ## _ ## ident2 ## _ ## ident3
#define VGM3(_1, _2, _3, Macro, ...) Macro
#define VCONCAT(...) \
    VGM3(__VA_ARGS__, V__CONCAT3, V__CONCAT2, V__CONCAT1)(__VA_ARGS__)

/* Some shortcuts for declaring types and functions */
#define VSNAME(Name)        VCONCAT(Name, t)
#define VFNAME(Name, FName) VCONCAT(Name, FName) 

/* some shorcuts for getting bits and pieces */
#define VELEM(Vec,  Index) (Vec)->elements[Index]
#define VSIZE(Vec)  (Vec)->size
#define VGTYPE(Vec) typeof(VELEM(Vec, 0))

/* note Var will be pointing to the element, it wont be the actual element */
#define VFOR(Var, Vec) \
    for (VGTYPE(Vec) *Var = &VELEM(Vec, 0); Var < &VELEM(Vec, VSIZE(Vec)); Var++)

#define VNONE

/* variants of vec declare */
#define _VEC_DECLARE_IMPL1(Type)                                               \
    VEC_IMPL_MACRO(VCONCAT(vec, VName), Type, VNONE, calloc, realloc, free)

#define _VEC_DECLARE_IMPL2(VName, Type)                                        \
    VEC_IMPL_MACRO(VName, Type, VNONE, calloc, realloc, free)

#define _VEC_DECLARE_IMPL3(VName, Type, Attr)                                  \
    VEC_IMPL_MACRO(VName, Type, Attr, calloc, realloc, free)

#define VEC_DECLARE_IMPL(...) \
    VGM3(__VA_ARGS__, _VEC_DECLARE_IMPL3, _VEC_DECLARE_IMPL2, _VEC_DECLARE_IMPL1)(__VA_ARGS__)

#define _VEC_DECLARE_H1(Type) \
    VEC_HEADER_MACRO(VCONCAT(vec, Type), Type, VNONE)
#define _VEC_DECLARE_H2(Name, Type) \
    VEC_HEADER_MACRO(Name, Type, VNONE)
#define _VEC_DECLARE_H3(Name, Type, Attr) \
    VEC_HEADER_MACRO(Name, Type, Attr)
    
#define VEC_DECLARE_HEADER(...) \
    VGM3(__VA_ARGS__, _VEC_DECLARE_H3, _VEC_DECLARE_H2, _VEC_DECLARE_H1)(__VA_ARGS__)

#define _VEC_DECLARE_L1(Type) \
    VEC_DECLARE_HEADER(VCONCAT(vec, Type), Type, static) \
    VEC_DECLARE_IMPL(VCONCAT(vec, Type), Type, static)

#define _VEC_DECLARE_L2(Name, Type) \
    VEC_DECLARE_HEADER(Name, Type, static) \
    VEC_DECLARE_IMPL(Name, Type, static)

#define VEC_DECLARE_LOCAL(...) \
    VGM3(__VA_ARGS__, error, _VEC_DECLARE_L2, _VEC_DECLARE_L1)(__VA_ARGS__)

/* 
 * the mega macros, called on by 
 *  - VEC_DECLARE_HEADER
 *  - VEC_DECLARE_IMPL
 *  - VEC_DECLARE_LOCAL
 * 
 * Takes a VName like vec_int
 *       a VType like int
 *       an FAttr like static (placed before every function)
 * 
 * calloc and free can be replaced with custom allocators
 */
#define VEC_HEADER_MACRO(VName, VType, FAttr)                                           \
    VEC_DECLARE_STRUCT(VName, VType)                                                    \
    VEC_DECLARE_PROTOTYPES(VName, VType, FAttr)

#define VEC_DECLARE_STRUCT(VName, VType)                                                \
typedef struct VName                                                                    \
{                                                                                       \
    VType *elements;                                                                    \
    size_t     size;                                                                    \
    size_t capacity;                                                                    \
} VSNAME(VName);

#define VEC_DECLARE_PROTOTYPES(VName, VType, FAttr)                                     \
FAttr void VFNAME(VName, init)(VSNAME(VName) *v);                                       \
FAttr VSNAME(VName) *VFNAME(VName, create)();                                           \
FAttr VSNAME(VName) *VFNAME(VName, realloc)(VSNAME(VName) *v);                          \
FAttr VSNAME(VName) *VFNAME(VName, push)(VSNAME(VName) *v, VType e);                    \
FAttr VType VFNAME(VName, pop)(VSNAME(VName) *v);                                       \
FAttr VType VFNAME(VName, top)(VSNAME(VName) *v);                                       \
FAttr VSNAME(VName) *VFNAME(VName, extend)(VSNAME(VName) *v, VType *array, size_t size);\
FAttr void VFNAME(VName, clear)(VSNAME(VName) *v);                                      \
FAttr void VFNAME(VName, destroy)(VSNAME(VName) *v);                                    \
FAttr void VFNAME(VName, free)(VSNAME(VName) *v);                                       \


#define VEC_IMPL_MACRO(VName, VType, FAttr, calloc, realloc, free)                      \
FAttr void VFNAME(VName, init)(VSNAME(VName) *v)                                        \
{                                                                                       \
    v->size = 0;                                                                        \
    v->elements =                                                                       \
        (VType *) calloc(v->capacity = 0x10, sizeof(VSNAME(VName)));                    \
                                                                                        \
    assert(v->elements);                                                                \
}                                                                                       \
                                                                                        \
FAttr VSNAME(VName) *VFNAME(VName, create)()                                            \
{                                                                                       \
    VSNAME(VName) *v = (VSNAME(VName) *) calloc(1, sizeof(VSNAME(VName)));              \
    assert(v);                                                                          \
    VFNAME(VName, init)(v);                                                             \
    return v;                                                                           \
}                                                                                       \
                                                                                        \
FAttr VSNAME(VName) *VFNAME(VName, realloc)(VSNAME(VName) *v)                           \
{                                                                                       \
    v->elements = (VType *) realloc(v->elements, (v->capacity *= 2) * sizeof(VType));   \
    assert(v->elements);                                                                \
    return v;                                                                           \
}                                                                                       \
                                                                                        \
FAttr VSNAME(VName) *VFNAME(VName, push)(VSNAME(VName) *v, VType e)                     \
{                                                                                       \
    if (v->size > v->capacity)                                                          \
        assert(0);                                                                      \
                                                                                        \
    if (v->size == v->capacity)                                                         \
        VFNAME(VName, realloc)(v);                                                      \
                                                                                        \
    v->elements[v->size++] = e;                                                         \
    return v;                                                                           \
}                                                                                       \
                                                                                        \
FAttr VType VFNAME(VName, pop)(VSNAME(VName) *v)                                        \
{                                                                                       \
    assert(v->size != 0);                                                               \
    return v->elements[--v->size];                                                      \
}                                                                                       \
                                                                                        \
FAttr VType VFNAME(VName, top)(VSNAME(VName) *v)                                        \
{                                                                                       \
    assert(v->size != 0);                                                               \
    return v->elements[v->size - 1];                                                    \
}                                                                                       \
                                                                                        \
FAttr VSNAME(VName) *VFNAME(VName, extend)(VSNAME(VName) *v, VType *array, size_t size) \
{                                                                                       \
    int i;                                                                              \
                                                                                        \
    for (i = 0; i < size; i++)                                                          \
        VFNAME(VName, push)(v, array[i]);                                               \
                                                                                        \
    return v;                                                                           \
}                                                                                       \
                                                                                        \
FAttr void VFNAME(VName, clear)(VSNAME(VName) *v)                                       \
{                                                                                       \
    v->size = 0;                                                                        \
}                                                                                       \
                                                                                        \
FAttr void VFNAME(VName, destroy)(VSNAME(VName) *v)                                     \
{                                                                                       \
    free(v->elements);                                                                  \
    v->size = v->capacity = 0;                                                          \
    v->elements = NULL;                                                                 \
}                                                                                       \
                                                                                        \
FAttr void VFNAME(VName, free)(VSNAME(VName) *v)                                        \
{                                                                                       \
    VFNAME(VName, destroy)(v);                                                          \
    free(v);                                                                            \
}                                                                                       \

#endif