
#include "_malloc.h"
#include "ft_printf.h"

static size_t
print_alloc(struct s_area *area, size_t zone_id)
{
	if (area == NULL)
	{
		switch (zone_id)
		{
			case 0:
				ft_printf("TINY: %p\n\n", area);
				break ;
			case 1:
				ft_printf("SMALL: %p\n\n", area);
				break ;
			case 2:
				ft_printf("LARGE: %p\n\n", area);
				break ;
			default:
				break ;
		}
		return 0;
	}
	size_t total_alloc_size;

	total_alloc_size = 0;
	while (area != NULL)
	{
		struct s_block	*block;
		
		switch (zone_id)
		{
			case 0:
				ft_printf("TINY: %p\n", area);
				break ;
			case 1:
				ft_printf("SMALL: %p\n", area);
				break ;
			case 2:
				ft_printf("LARGE: %p\n", area);
				break ;
			default:
				return 0;
		}
		block = area->data_ptr;
		while (block->prev_block != NULL || block == area->data_ptr)
		{
			if (block->n_free_list == NULL)
			{
				// block is in use
				ft_printf("%p - %p : %u bytes\n", &block->data, &block->data + block->size, block->size);
				total_alloc_size += block->size;
			}
			else
			{
				ft_printf("%p - %p : %u free bytes\n", &block->data, &block->data + block->size, block->size);
			}
			block = (struct s_block *)(&block->data + block->size); 
		}
		ft_printf("\n");
		area = area->next;
	}
	return total_alloc_size;
}

void show_alloc_mem(void)
{
	size_t total_alloc_size;

	ft_printf("HEAP:\n\n");
	total_alloc_size = 0;
	for (size_t i = 0; i < 3; i++)
	{
		total_alloc_size += print_alloc(heap.area[i], i);
	}
	ft_printf("Total: %u bytes in use\n", total_alloc_size);
}