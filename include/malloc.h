#ifndef __MALLOC_H__
# define __MALLOC_H__

#include <stddef.h>

// we need to include stdlib to overwrite fonction
#include <stdlib.h>

void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);
void show_alloc_mem(void);

#endif /* __MALLOC_H__ */