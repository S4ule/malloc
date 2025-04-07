

#include "ft_printf.h"
	// ft_printf()

#include "malloc.h"
	// free()
	// malloc()

#include "_malloc.h"
	// struct s_block
	// struct s_area
	// struct s_heap
	// extern struct s_heap heap
	// update_free_list()
	// revmove_from_free_list()
	// find_area()
	// find_block_in_area()
	// ft_memcpy()
	// BLOCK_INFO_SIZE

static struct s_block *
_try_to_expend_block(struct s_block *block, size_t request_size, size_t zone_id)
{
	if (request_size <= block->size)
	{
		// we do nothing
		// no need to alloced a new block for a smaller request_size
		// we just continue with the one we already started to use
		return block;
	}
	struct s_block *next_block;

	next_block = (struct s_block *)(&block->data + block->size);
	if (next_block->prev_block == NULL || next_block->n_free_list == NULL)
	{
		// there is no free block to try to expland
		return NULL;
	}
	
	// calculate final_block_size
	// we don't add BLOCK_INFO_SIZE in the final_block_size because is not in the block imself
	size_t final_block_size;
	
	final_block_size =  request_size + get_padding_size(&(block->data) + request_size + BLOCK_INFO_SIZE);

	// we save next_next_block value
	struct s_block *next_next_block;

	next_next_block = (struct s_block *)(&next_block->data + next_block->size);
	if ((block->size + next_block->size + BLOCK_INFO_SIZE) == request_size)
	{
		// we merge both block togeter
		revmove_from_free_list(&heap.free_list[zone_id], next_block);

		block->size = (block->size + next_block->size + BLOCK_INFO_SIZE);
		if (next_next_block->prev_block != NULL)
		{
			next_next_block->prev_block = block;
		}
		return block;
	}

	if (final_block_size < request_size)
	{
		// even with the next_block size it's not enong
		// we just alloc a new block
		return NULL;
	}
	// at this point we have a next_block in the free_list and he have space
	// but we need to move the next_block and add it to free_list

	// save the next_block size and we change the block size
	size_t next_block_size;

	next_block_size = (block->size + next_block->size) - final_block_size;
	block->size = final_block_size;

	// we remove it next_block from free_list
	revmove_from_free_list(&heap.free_list[zone_id], next_block);

	// setup the next_block
	next_block = (struct s_block *)(&block->data + final_block_size);
	next_block->prev_block = block;
	next_block->size = next_block_size;
	next_block->n_free_list = NULL;

	// finaly we add next_block in free_list and set next_next_block value
	heap.free_list[zone_id] = update_free_list(heap.free_list[zone_id], next_block);

	if (next_next_block->prev_block != NULL)
	{
		next_next_block->prev_block = next_block;
	}
	return block;
}


void *
realloc(void *ptr, size_t request_size)
{
	if (ptr == NULL)
	{
		// If ptr is equal to NULL, then the call is equivalent to malloc(request_size)
		return malloc(request_size);
	}
	else if (request_size == 0)
	{
		// If request_size is equal to zero, and ptr is not NULL, then the call is equivalent to free(ptr)
		free(ptr);
		return NULL;
	}

	struct s_area *area;
	struct s_block *block;
	size_t zone_id;

	area = find_area(ptr, &zone_id);
	if (area == NULL)
	{
		ft_printf("error: realloc(): ptr is not a valid ptr\n");
		return NULL;
	}

	block = find_block_in_area(ptr, area);
	if (block == NULL || block->n_free_list != NULL)
	{
		ft_printf("error: realloc(): ptr is not a valid ptr anymore\n");
		return NULL;
	}

	struct s_block *selected_block;

	selected_block = _try_to_expend_block(block, request_size, zone_id);
	if (selected_block == NULL)
	{
		void *new_ptr;

		new_ptr = malloc(request_size);
		if (new_ptr == NULL)
		{
			free(ptr);
			return NULL;
		}
		ft_memcpy(new_ptr, &block->data, block->size);
		free(ptr);
		return new_ptr;
	}
	return &(selected_block->data);
}