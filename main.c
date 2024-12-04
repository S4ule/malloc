
#include "malloc.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>


int
main(void)
{
	size_t size = 1;
	char *ptr;

	for (size_t top = 1; top < 200; top++)
	{
		ptr = malloc(sizeof(size_t) * size * top);
		if (ptr == NULL)
		{
			write(1, "malloc fail\n", strlen("malloc fail\n"));
			return 0;
		}
		write(1, "malloc success\n", strlen("malloc success\n"));
		show_alloc_mem();
		free(ptr);
		write(1, "\n\n", 2);
	}
	return 0;
}
