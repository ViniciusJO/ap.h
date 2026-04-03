#ifndef __ARRAYLIST_H__
#define __ARRAYLIST_H__
#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include <stddef.h>

struct _IO_FILE;
typedef struct _IO_FILE FILE;

/* Standard streams.  */
extern FILE *stdin;		  /* Standard input stream.        */
extern FILE *stdout;		/* Standard output stream.       */
extern FILE *stderr;		/* Standard error output stream. */
/* C89/C99 say they're macros.  Make them happy.  */
#define stdin stdin
#define stdout stdout
#define stderr stderr

#ifndef MAX
#define MAX(A, B) (((A) > (B)) ? (A) : (B))
#endif

#ifndef AL_ASSERT
#include <assert.h>
#define AL_ASSERT assert
#endif//AL_ASSERT

#ifndef AL_INIT_CAP
#define AL_INIT_CAP 256
#endif//AL_INIT_CAP

#ifndef AL_REALLOC
extern void *realloc(void *__ptr, size_t __size) __THROW __attribute_warn_unused_result__ __attribute_alloc_size__ ((2));
#define AL_REALLOC realloc
#endif//AL_REALLOC
#define AL_REALLOC_TYPED(ptr, count) ((__typeof__(ptr)) AL_REALLOC(ptr, sizeof(*(ptr)) * (count)))
      
#ifndef AL_FREE
extern void free(void *__ptr) __THROW;
#define AL_FREE free
#endif//AL_FREE

#ifndef AL_MEMCPY
extern void *memcpy(void *__restrict __dest, const void *__restrict __src, size_t __n) __THROW __nonnull ((1, 2));
#define AL_MEMCPY memcpy
#endif//AL_MEMCPY

#ifndef AL_FPRINTF
extern int fprintf(FILE *__restrict stream, const char *__restrict format, ...) __nonnull ((1));
#define AL_FPRINTF fprintf
#endif//AL_FPRINTF

#ifndef AL_FFLUSH
extern int fflush (FILE *__stream);
#define AL_FFLUSH fflush
#endif//AL_FFLUSH

#ifndef AL_EXIT
extern void exit(int status);
#define AL_EXIT exit
#endif//AL_EXIT

#define al_define(type, name) typedef struct { type *items; size_t count; size_t capacity; } name

#define al_reserve(al, expected_capacity) do {                                      \
  if ((expected_capacity) > (al)->capacity) {                                       \
    if ((al)->capacity == 0) (al)->capacity = AL_INIT_CAP;                          \
    while ((expected_capacity) > (al)->capacity) (al)->capacity *= 2;               \
    (al)->items = AL_REALLOC_TYPED((al)->items, (al)->capacity);   \
    AL_ASSERT((al)->items != NULL);                                                 \
  }                                                                                 \
} while (0)

/*
#define al_resize(al, new_size) do {  \
  al_reserve((al), new_size);       \
  (al)->count = (new_size);         \
} while (0)
*/

// Append an item to an array list
#define al_append(al, item) do {       \
  al_reserve((al), (al)->count + 1);   \
  (al)->items[(al)->count++] = (item); \
} while (0)

// Append several items to a dynamic array
#define al_append_many(al, new_items, new_items_count) do {  \
  al_reserve((al), (al)->count + (new_items_count));         \
  memcpy(                                                    \
    (al)->items + (al)->count,                               \
    (new_items),                                             \
    (new_items_count)*sizeof(*(al)->items)                   \
  );                                                         \
  (al)->count += (new_items_count);                          \
} while (0)

#define al_insert(al, pos, x) do {                                                                                  \
  if(0 == (al)->capacity) al_reserve((al), AL_INIT_CAP);                                                            \
  size_t position = ((pos) < 0) ? (al)->count + (pos) : (size_t)(pos);                                              \
  if(position > (al)->count) {                                                                                      \
    AL_FPRINTF(                                                                                                     \
        stderr,                                                                                                     \
        "position %d out of bounds [%ld, %ld]\n",                                                                   \
        (pos), -(al)->count, (al)->count                                                                            \
    );                                                                                                              \
    AL_EXIT(1);                                                                                                     \
  }                                                                                                                 \
  al_reserve((al), (al)->count+1);                                                                                  \
  if(position < (al)->count)                                                                                        \
    AL_MEMCPY(&(al)->items[position+1], &(al)->items[position], ((al)->count - position)*sizeof(*(al)->items));     \
    (al)->items[position] = x;                                                                                      \
  (al)->count += 1;                                                                                                 \
} while(0)

#define al_insert_many(al, pos, new_items, new_items_count) do {                                               \
  if(0 == (al)->capacity) al_reserve((al), AL_INIT_CAP);                                                            \
  size_t position = ((pos) < 0) ? (al)->count + (pos) : (size_t)(pos);                                         \
  if(position > (al)->count) {                                                                                 \
    AL_FPRINTF(                                                                                                 \
        stderr,                                                                                                  \
        "position %d out of bounds [%ld, %ld]\n",                                                                \
        (pos), -(al)->count, (al)->count                                                                     \
    );                                                                                                           \
    AL_EXIT(1);                                                                                                     \
  }                                                                                                              \
  al_reserve((al), (al)->count+new_items_count);                                                             \
  if(position < (al)->count)                                                                                   \
    AL_MEMCPY(&(al)->items[position+new_items_count], &(al)->items[position], ((al)->count - position)*sizeof(*(al)->items));     \
  AL_MEMCPY(&(al)->items[position], new_items, new_items_count*sizeof(*(al)->items));                                         \
  (al)->count += new_items_count;                                                                              \
} while(0)



#define al_first(al) (al)->items[(AL_ASSERT((al)->count > 0), 0)]
#define al_last(al) (al)->items[(AL_ASSERT((al)->count > 0), (al)->count-1)]

#define al_pop(al) ((al)->count--, (al)->items[(al)->count])
#define al_pop_many(al, _count) ((al)->count -= _count, &(al)->items[(al)->count])

#define al_remove_unordered(al, pos) do {        \
  size_t position = ((pos) < 0) ? (al)->count + (pos) : (size_t)(pos);                                         \
  if(position > (al)->count) {                                                                                 \
    AL_FPRINTF(                                                                                                 \
        stderr,                                                                                                  \
        "position %d out of bounds [%ld, %ld]\n",                                                                \
        (pos), -(al)->count, (al)->count                                                                     \
    );                                                                                                           \
    AL_EXIT(1);                                                                                                     \
  }                                                                                                              \
  AL_ASSERT(position < (al)->count);                  \
  (al)->items[position] = (al)->items[--(al)->count]; \
} while(0)

#define al_remove(al, pos) do {                                                                                \
  if(0 == (al)->capacity) {                                                                                            \
    al_reserve(al, AL_INIT_CAP);                                                                            \
    break;                                                                                                       \
  }                                                                                                              \
  size_t position = ((pos) < 0) ? (al)->count + (pos) : (size_t)(pos);                                         \
  if(position > (al)->count) {                                                                                 \
    AL_FPRINTF(                                                                                                 \
        stderr,                                                                                                  \
        "position %d out of bounds [%ld, %ld]\n",                                                                \
        (pos), -(al)->count, (al)->count                                                                     \
    );                                                                                                           \
    AL_EXIT(1);                                                                                                     \
  }                                                                                                              \
  AL_MEMCPY(&(al)->items[position], &(al)->items[position+1], ((al)->count - position)*sizeof(*(al)->items));                     \
  (al)->count--;                                                                                               \
} while(0)

#define al_remove_many(al, pos, _count) do {                                                                   \
  if(0 == (al)->count) {                                                                                            \
    al_reserve(al, AL_INIT_CAP);                                                                            \
    break;                                                                                                       \
  }                                                                                                              \
  size_t position = ((pos) < 0) ? (al)->count + (pos) : (size_t)(pos);                                         \
  if(position > (al)->count) {                                                                                 \
    AL_FPRINTF(                                                                                                 \
        stderr,                                                                                                  \
        "position %d out of bounds [%ld, %ld]\n",                                                                \
        (pos), -(al)->count, (al)->count                                                                     \
    );                                                                                                           \
    AL_EXIT(1);                                                                                                     \
  }                                                                                                              \
  size_t quant = ((al)->count - position < _count) ? (al)->count - position : _count;                        \
  AL_MEMCPY(&(al)->items[position], &(al)->items[position+_count], ((al)->count - position - quant)*sizeof(*(al)->items));        \
  (al)->count -= _count;                                                                                       \
} while(0)

#define al_free(al) do { \
  AL_FREE((al)->items); \
  (al)->items = NULL; \
} while(0)


// Foreach over Dynamic Arrays. Example:
// ```c
// typedef struct {
//     int *items;
//     size_t count;
//     size_t capacity;
// } Numbers;
//
// Numbers xs = {0};
//
// al_append(&xs, 69);
// al_append(&xs, 420);
// al_append(&xs, 1337);
//
// al_foreach(int, x, &xs) {
//     // `x` here is a pointer to the current element. You can get its index by taking a difference
//     // between `x` and the start of the array which is `x.items`.
//     size_t index = x - xs.items;
//     nob_log(INFO, "%zu: %d", index, *x);
// }
// ```

#define al_foreach(it, al) \
  if(NULL != (al)->items) \
  for (size_t it##_idx = 0; 0 == it##_idx;) \
  for(__typeof__((al)->items) it = (al)->items;  it##_idx < (al)->count; ++it##_idx, ++it)


#define FMT(x) _Generic((x), \
    /* signed integers */ \
    char: "%hhd", \
    signed char: "%hhd", \
    short: "%hd", \
    int: "%d", \
    long: "%ld", \
    long long: "%lld", \
    \
    /* unsigned integers */ \
    unsigned char: "%hhu", \
    unsigned short: "%hu", \
    unsigned int: "%u", \
    unsigned long: "%lu", \
    unsigned long long: "%llu", \
    \
    /* floating point */ \
    float: "%f", \
    double: "%f", \
    long double: "%Lf", \
    \
    /* character / string */ \
    char*: "%s", \
    const char*: "%s", \
    \
    /* pointers */ \
    void*: "%p", \
    const void*: "%p", \
    \
    /* fallback */ \
    default: "<unknown>" \
)

#define al_print(al,...) do { \
  /*AL_FPRINTF(stdout, #al "[%ld/%ld] = ", (al)->count, (al)->capacity); */\
  AL_FPRINTF(stdout, "{ "); \
  al_foreach(i, al) { \
    AL_FPRINTF(stdout, FMT(*i), *i); \
    AL_FPRINTF(stdout, "%s", (al)-> count-1 != i_idx ? ", " : "" ); \
  } else AL_FPRINTF(stdout, "NULL\n"); \
  AL_FPRINTF(stdout, " }" __VA_ARGS__); \
} while(0)

#define al_print_object(al) do { \
  AL_FPRINTF(stdout, "{\n  .count = %ld,\n  .capacity = %ld,\n  .items = ", (al)->count, (al)->capacity); \
  al_print(al); \
  AL_FPRINTF(stdout, "\n}\n"); \
} while(0)





#ifndef ARRAY
#define ARRAY(...) { __VA_ARGS__ }
#endif//ARRAY

#define al_expect_literal(element_type, al, expected) do { \
  element_type expected_elements[] = expected; \
  size_t expected_size = sizeof(expected_elements)/sizeof(*expected_elements); \
  AL_ASSERT((al)->count == expected_size); \
  al_foreach(ael_i, al) \
    AL_ASSERT((al)->items[ael_i_idx] == expected_elements[ael_i_idx]); \
} while(0)




#ifdef __cplusplus
}
#endif//__cplusplus
#endif//__ARRAYLIST_H__

