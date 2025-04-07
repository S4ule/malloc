
#include "ft_printf.h"
	// ft_printf()

# include <sys/mman.h>
	// int munmap()

# include <stddef.h>
	// NULL

#include "_malloc.h"
	// struct s_block
	// struct s_area
	// struct s_heap
	// extern struct s_heap heap
	// update_free_list()
	// revmove_from_free_list()
	// find_area()
	// find_block_in_area()
	// BLOCK_INFO_SIZE

static void
free_the_block(struct s_block *block, size_t zone_id)
{
	struct s_block *next_block;

	next_block = (struct s_block *)(&block->data + block->size);
	if (next_block->prev_block != NULL && next_block->n_free_list != NULL && \
		block->prev_block != NULL && block->prev_block->n_free_list != NULL )
	{
		// we can merge the next block and the prev block with this one
		block->prev_block->size += BLOCK_INFO_SIZE * 2 + block->size + next_block->size;

		// prev_block is already in free_list we don't need to add it
		// but next_block is also in free_list, we need to find it and remove it
		revmove_from_free_list(&heap.free_list[zone_id], next_block);

		// we need to change prev_block from the next of next_block
		struct s_block *next_next_block;

		next_next_block = (struct s_block *)(&next_block->data + next_block->size);
		if (next_next_block->prev_block != NULL)
		{
			// don't forget if prev_block is NULL that mark the end of the area
			// if not NULL we change it
			next_next_block->prev_block = block;
		}
	}
	else if (next_block->prev_block != NULL && next_block->n_free_list != NULL)
	{
		// we can merge the next block with this one
		block->size += BLOCK_INFO_SIZE + next_block->size;
		
		// next_block is in free_list
		// we need to remove it and add the new block in it
		revmove_from_free_list(&heap.free_list[zone_id], next_block);
		heap.free_list[zone_id] = update_free_list(heap.free_list[zone_id], block);

		// we need to change prev_block from the next of next_block
		struct s_block *next_next_block;

		next_next_block = (struct s_block *)(&next_block->data + next_block->size);
		if (next_next_block->prev_block != NULL)
		{
			// don't forget if prev_block is NULL that mark the end of the area
			// if not NULL we change it
			next_next_block->prev_block = block;
		}
	}
	else if (block->prev_block != NULL && block->prev_block->n_free_list != NULL)
	{
		// we can merge the prev block with this one
		block->prev_block->size += BLOCK_INFO_SIZE + block->size;
		
		// prev_block is already in free_list so we don't need to add it

		// but we still need to change prev_block from the next_block
		if (next_block->prev_block != NULL)
		{
			// don't forget if prev_block is NULL that mark the end of the area
			next_block->prev_block = block->prev_block;
		}
	}
	else
	{
		// we can merge nothing
		heap.free_list[zone_id] = update_free_list(heap.free_list[zone_id], block);
	}
	return ;
}

static void
free_all_area(void)
{
	for (size_t i = 0; i < 3; i++)
	{
		struct s_area *area;

		area = heap.area[i];
		while (area != NULL)
		{
			struct s_area *next_area;

			next_area = area->next;
			munmap(area, area->size);
			area = next_area;
		}
		heap.area[i] = NULL;
		heap.free_list[i] = NULL;
	}
	return ;
}

void
free(void * ptr)
{
	if (ptr == NULL)
	{
		// do nothing
		return ;
	}

	struct s_area *area;
	size_t zone_id;

	area = find_area(ptr, &zone_id);
	if (area == NULL)
	{
		ft_printf("error: free(): ptr is not a valid ptr\n");
		return ;
	}

	struct s_block *block;

	block = find_block_in_area(ptr, area);
	if (block == NULL || block->n_free_list != NULL)
	{
		ft_printf("error: free(): ptr is not a valid ptr anymore\n");
		return ;
	}
	free_the_block(block, zone_id);
	heap.n_alloced_block -= 1;
	if (heap.n_alloced_block == 0)
	{
		// in theory we need to free_all_area at exit and not in such case
		// but the 42 subject for this is grabage
		free_all_area();
	}
	return ;
}