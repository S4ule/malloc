
#include <sys/mman.h>
	// void *mmap()

#include <stddef.h>
	// NULL

#include <unistd.h>
	// sysconf()

#include "_malloc.h"
	// struct s_block
	// struct s_area
	// struct s_heap
	// update_free_list()
	// revmove_from_free_list()
	// BLOCK_INFO_SIZE
	// AREA_INFO_SIZE
	// M_ALIGN
	// N_BLOCK_IN_AREA
	// LARGE_ALLOC
	// SMALL_ALLOC
	// TINY_ALLOC

struct s_heap heap = {
	.area = {NULL, NULL, NULL},
	.free_list = {NULL, NULL, NULL},
	.n_alloced_block = 0,
	.free_list_end = NULL
};

// return the size of the block with all the block info header
static size_t
get_block_size(size_t alloced_block_size)
{
	return BLOCK_INFO_SIZE + alloced_block_size;
}

// return the size of the area for N_BLOCK_IN_AREA
static size_t
get_area_size(size_t alloced_block_size)
{
	return AREA_INFO_SIZE + (get_block_size(alloced_block_size) + M_ALIGN - 1) * N_BLOCK_IN_AREA;
}

// return the size of the area for N_BLOCK_IN_AREA
static size_t
get_large_area_size(size_t alloced_block_size)
{
	return AREA_INFO_SIZE + (get_block_size(alloced_block_size) + BLOCK_INFO_SIZE + sizeof(struct s_block *) + M_ALIGN - 1);
}

static struct s_area *
create_new_area(size_t max_block_size)
{
	size_t	page_size;
	size_t	area_size;
	struct s_area	*area;

#ifdef __OSX__
	page_size = getpagesize();
#else
	page_size = sysconf(_SC_PAGESIZE);
#endif
	if (page_size == 0)
	{
		return NULL;
	}

	if (max_block_size > SMALL_ALLOC)
	{
		area_size = (get_large_area_size(max_block_size) / page_size + 1) * page_size;
	}
	else
	{
		area_size = (get_area_size(max_block_size) / page_size + 1) * page_size;
	}


	area = mmap(NULL, area_size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (area == MAP_FAILED)
	{
		return NULL;
	}

	// init area info
	area->size = area_size;
	area->next = NULL;

	struct s_block *data;
	size_t padding_size;

	data = (struct s_block *)(&(area->data));
	data = (struct s_block *)((unsigned char *)data + BLOCK_INFO_SIZE);
	padding_size = get_padding_size(data);
	data = (struct s_block *)((unsigned char *)data + padding_size - BLOCK_INFO_SIZE);

	// set the start to data aray in area info
	area->data_ptr = data; 

	// setup the first block of the area
	size_t block_size;

	block_size = area_size - (AREA_INFO_SIZE + padding_size + BLOCK_INFO_SIZE + sizeof(struct s_block *));
	data->prev_block = NULL;
	data->size = block_size;
	data->n_free_list = NULL;
	data = (struct s_block *)(&data->data + block_size);
	data->prev_block = NULL;

	return area;
}


static struct s_block *
get_free_block_in_free_list(struct s_block *free_list, size_t request_size)
{
	size_t final_block_size;

	while (free_list != NULL)
	{
		// we add BLOCK_INFO_SIZE in the final_block_size because we need to be sure we have space for another block
		final_block_size =  request_size + BLOCK_INFO_SIZE + get_padding_size(&(free_list->data) + request_size + BLOCK_INFO_SIZE);
		if (request_size == free_list->size || free_list->size >= final_block_size)
		{
			return free_list;
		}
		free_list = free_list->n_free_list;
	}
	return NULL;
}

static struct s_block *
use_free_block(struct s_block *block, size_t request_size, struct s_block **free_list)
{
	if (block->size == request_size)
	{
		// In that case we just set the block as alloced
		block->n_free_list = NULL;

		// Remove the block from the free_list and return it
		revmove_from_free_list(free_list, block);
		return block;
	}

	// Overwise we need to create a new block
	size_t final_block_size;

	// we don't add BLOCK_INFO_SIZE in the final_block_size because is not in the block imself
	final_block_size =  request_size + get_padding_size(&(block->data) + request_size + BLOCK_INFO_SIZE);

	// we setup the new block
	struct s_block *new_block;

	new_block = (struct s_block *)(&(block->data) + final_block_size);
	new_block->prev_block = block;
	new_block->size = block->size - (final_block_size + BLOCK_INFO_SIZE);
	new_block->n_free_list = NULL;

	// Remove the block from the free_list
	revmove_from_free_list(free_list, block);

	// we add the new_block in the free_list of the area
	*free_list = update_free_list(*free_list, new_block);

	// we setup the new_block_tail
	struct s_block *new_block_tail;

	new_block_tail = (struct s_block *)(&(new_block->data) + new_block->size);
	if (new_block_tail->prev_block != NULL)
	{
		// Don't forget prev_block should be NULL if this is the last block of the area
		new_block_tail->prev_block = new_block;
	}

	// Finaly we setup the block imself and return it
	block->size = final_block_size;
	block->n_free_list = NULL;
	return block;
}

static struct s_block *
get_block(size_t request_size, size_t max_block_size, size_t zone_id)
{
	if (heap.area[zone_id] == NULL)
	{
		struct s_area *new_area;

		new_area = create_new_area(max_block_size);
		if (new_area == NULL)
		{
			// can't recover from that...
			return NULL;
		}
		new_area->next = heap.area[zone_id];
		heap.area[zone_id] = new_area; // set the new area on heap
		heap.free_list[zone_id] = update_free_list(heap.free_list[zone_id], new_area->data_ptr);
	}

	struct s_block *selected_block;

	selected_block = get_free_block_in_free_list(heap.free_list[zone_id], request_size);
	if (selected_block == NULL)
	{
		// we need more space
		struct s_area *new_area;

		new_area = create_new_area(max_block_size);
		if (new_area == NULL)
		{
			// we can still can try to recover from here by freeing free area
			// but to do that we need to keep track of the nummber of block_alloced in area
			// and this it's probably not optimal to do for sutch a small gain in that weird case
			return NULL;
		}
		new_area->next = heap.area[zone_id];
		heap.area[zone_id] = new_area; // set the new area on heap
		heap.free_list[zone_id] = update_free_list(heap.free_list[zone_id], new_area->data_ptr);

		selected_block = get_free_block_in_free_list(heap.free_list[zone_id], request_size);
		if (selected_block == NULL)
		{
			// can't recover from that...
			return NULL;
		}
	}

	// at this point we should have a selected_block
	selected_block = use_free_block(selected_block, request_size, &(heap.free_list[zone_id]));
	heap.n_alloced_block += 1;
	return selected_block;
}

static struct s_block *
get_large_block(size_t request_size)
{
	struct s_area	*new_area;
	struct s_block	*selected_block;

	new_area = create_new_area(request_size);
	if (new_area == NULL)
	{
		return NULL;
	}
	new_area->next = heap.area[2];
	heap.area[2] = new_area;

	// setup block
	selected_block = new_area->data_ptr;
	selected_block->n_free_list = NULL;
	revmove_from_free_list(&(heap.free_list[2]), selected_block);

	heap.n_alloced_block += 1;
	return selected_block;
}

void *
malloc(size_t size)
{
	struct s_block *selected_block;

	if (size <= TINY_ALLOC)
	{
		selected_block = get_block(size, TINY_ALLOC, 0);
	}
	else if (size <= SMALL_ALLOC)
	{
		selected_block = get_block(size, SMALL_ALLOC, 1);
	}
	else
	{
		selected_block = get_large_block(size);
	}

	if (selected_block == NULL)
	{
		return NULL;
	}
	return &(selected_block->data);
}