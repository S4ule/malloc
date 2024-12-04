
#include "_malloc.h"

static void
print_dec(size_t n)
{
	char	c;

	if (n > 9)
	{
		print_dec(n / 10);
		n %= 10;
	}
	c = n + '0';
	write(1, &c, 1);
}

static void
print_hexa(size_t n)
{
	char	c;

	if (n > 15)
	{
		print_hexa(n / 16);
		n %= 16;
	}
	if (n < 10)
		c = n + '0';
	else
		c = n + 'A' - 10;
	write(1, &c, 1);
}

static void
print_address(size_t n)
{
	write(1, "0x", 2);
	print_hexa(n);
}

static size_t
print_alloc(struct s_area *area)
{
	unsigned char	*ptr;
	unsigned char	*max_area;
	struct s_block	*block;
	size_t			total_alloc_size;

	print_address((size_t)area);
	write(1, "\n", 1);

	total_alloc_size = 0;
	while (area != NULL)
	{
		block = &(area->block);
		max_area = (unsigned char *)area + area->area_size;
		while ((size_t)block < (size_t)max_area)
		{	
			ptr = &(block->data);
			if ((block->size & ALLOCED_FLAG) == ALLOCED_FLAG)
			{
				print_address((size_t)ptr);
				write(1, " - ", 3);
				ptr += block->size - ALLOCED_FLAG;
				print_address((size_t)ptr);
				write(1, " : ", 3);
				print_dec(block->size - ALLOCED_FLAG);
				write(1, " bytes\n", 7);
				total_alloc_size += block->size - ALLOCED_FLAG;
			}
			else
				ptr += block->size - ALLOCED_FLAG;
			ptr += BLOCK_TAIL_SIZE;
			block = (struct s_block *)ptr;
		}
		area = area->next_area;
	}
	write(1, "\n", 1);
	return total_alloc_size;
}

void show_alloc_mem(void)
{
	size_t total_alloc_size;

	total_alloc_size = 0;
	write(1, "TINY: ", 6);
	total_alloc_size += print_alloc(heap.area[0]);

	write(1, "SMALL: ", 7);
	total_alloc_size += print_alloc(heap.area[1]);

	write(1, "LARGE: ", 7);
	total_alloc_size += print_alloc(heap.area[2]);

	write(1, "Total : ", 8);
	print_dec(total_alloc_size);
	write(1, " bytes\n", 8);
}