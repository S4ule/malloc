#ifndef __MALLOC_H__
# define __MALLOC_H__

#include <stddef.h>

void *malloc(size_t size);
void free(void *ptr);
void show_alloc_mem(void);

#endif /* __MALLOC_H__ */