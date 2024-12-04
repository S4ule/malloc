
#include "_malloc.h"

struct s_heap heap = {
	.area = {NULL, NULL, NULL},
	.free_list = {NULL, NULL, NULL},
};

static struct s_area *
create_new_area(size_t size)
{
	size_t	page_size;
	size_t	area_size;
	struct s_area	*area;
	struct s_block	*block;
	unsigned char	*ptr;

#ifdef __OSX__
	page_size = getpagesize();
#else
	page_size = sysconf(_SC_PAGESIZE);
#endif
	if (page_size == 0)
		return NULL;

	area_size = (get_area_size(size) / page_size + 1) * page_size;

	area = mmap(NULL, area_size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (area == MAP_FAILED)
		return NULL;

	// setup area
	area->area_size = area_size;
	area->n_block = 0;
	area->next_area = NULL;
	area->null_ptr = NULL;

	// setup header of first block
	block = &(area->block);
	block->size = area_size - (AREA_INFO_SIZE + BLOCK_INFO_SIZE);
	block->n_free_list = NULL;

	// setup tail of first block
	ptr = &(block->data);
	ptr += block->size;
	{
		unsigned char **ptr_ptr;

		ptr_ptr = (unsigned char **)ptr;
		*ptr_ptr = (unsigned char *)block;
	}

	return area;
}

static struct s_block *
use_block(struct s_block *block, size_t size)
{
	size_t			padding_size;
	size_t			final_size;
	size_t			not_used_size;
	unsigned char	*data_start;
	unsigned char	*e_block_ptr;

	// setup block info
	data_start = &(block->data);
	e_block_ptr = (data_start + size);
	padding_size = get_padding_size(e_block_ptr + BLOCK_INFO_SIZE);
	final_size = padding_size + size;
	not_used_size = block->size - final_size;
	e_block_ptr = e_block_ptr + padding_size;

	if (not_used_size <= BLOCK_INFO_SIZE)
	{
		e_block_ptr = e_block_ptr + not_used_size;
		final_size += not_used_size;
		{
			unsigned char **ptr_ptr;

			ptr_ptr = (unsigned char **)e_block_ptr;
			*ptr_ptr = (unsigned char *)block;
		}
		not_used_size = 0;
		block->n_free_list = NULL;

	}
	else
	{
		struct s_block	*free_block;
		unsigned char	*e_free_block;

		free_block = (struct s_block *)(e_block_ptr + BLOCK_TAIL_SIZE);
		free_block->size = not_used_size - BLOCK_INFO_SIZE;
		
		e_free_block = (&(free_block->data)) + free_block->size;

		{
			unsigned char **ptr_ptr;

			ptr_ptr = (unsigned char **)e_free_block;
			*ptr_ptr = (unsigned char *)free_block;
		}

		free_block->n_free_list = block->n_free_list;
		block->n_free_list = free_block;
	}
	block->size = final_size | ALLOCED_FLAG;
	return block;
}

static struct s_block *
get_block_in_free_list(struct s_block **free_list, size_t size, struct s_block ***p_block)
{
	if (*free_list == NULL)
		return NULL;

	struct s_block *ptr_list;

	ptr_list = *free_list;
	while (ptr_list != NULL)
	{
		if (ptr_list->size >= size)
		{
			(*p_block) = free_list;
			return ptr_list;
		}
		ptr_list = ptr_list->n_free_list;
	}
	(*p_block) = NULL;
	return NULL;
}

static struct s_area *
find_block_area(struct s_block *block, struct s_area *sp)
{
	unsigned char *area_ptr;
	unsigned char *block_ptr;

	block_ptr = (unsigned char *)block;
	area_ptr = (unsigned char *)sp;
	while (area_ptr != NULL)
	{
		if (area_ptr < block_ptr && area_ptr + sp->area_size > block_ptr)
		{
			return sp;
		}
		sp = sp->next_area;
		area_ptr = (unsigned char *)sp;
	}
	return NULL;
}

void *
get_block(size_t size, size_t alloc_size, size_t zone_id)
{
	struct s_block	**p_block;
	struct s_block	*block;
	struct s_area	*area;

	block = NULL;
	block = get_block_in_free_list(&(heap.free_list[zone_id]), size, &p_block);
	if (block == NULL)
	{
		area = create_new_area(alloc_size);
		if (area == NULL)
			return NULL;

		area->next_area = heap.area[zone_id];
		heap.area[zone_id] = area;

		block = (struct s_block *)(&(area->block));
		block->n_free_list = heap.free_list[zone_id];
		heap.free_list[zone_id] = block;
		p_block = &heap.free_list[zone_id];
	}
	block = use_block(block, size);

	if (p_block != NULL && *p_block != NULL)
	{
		if (block->n_free_list == NULL)
			(*p_block) = NULL;
		else
			(*p_block) = block->n_free_list;
		block->n_free_list = NULL;
	}

	area = find_block_area(block, heap.area[zone_id]);
	if (area == NULL)
		return NULL;
	area->n_block += 1;

	return &(block->data);
}

void *
malloc(size_t size)
{
	if (size <= TINY_ALLOC)
	{
		return get_block(size, TINY_ALLOC, 0);
	}
	else if (size <= SMALL_ALLOC)
	{
		return get_block(size, SMALL_ALLOC, 1);
	}
	else if (size <= LARGE_ALLOC)
	{
		return get_block(size, LARGE_ALLOC, 2);
	}

	return NULL;
}