
#include "_malloc.h"

// return how much padding we need to be aline with M_ALIGN
size_t
get_padding_size(void *addr)
{
	size_t res;

	res = M_ALIGN - (unsigned long long)addr % M_ALIGN;

	if (res == M_ALIGN)
	{
		return 0;
	}
	return res;
}

void *
ft_memcpy(void *dest, const void *src, size_t n)
{
	unsigned char	*ptr_dst;
	unsigned char	*ptr_src;

	ptr_dst = (unsigned char *)dest;
	ptr_src = (unsigned char *)src;
	while (n > 0)
	{
		*ptr_dst = *ptr_src;
		ptr_dst++;
		ptr_src++;
		n--;
	}
	return (dest);
}

struct s_block *
update_free_list(struct s_block *free_list, struct s_block *new_block)
{
	if (free_list == NULL || free_list->n_free_list == NULL)
	{
		new_block->n_free_list = (struct s_block *)(&heap.free_list_end);
	}
	else
	{
		new_block->n_free_list = free_list;
	}
	free_list = new_block;
	return free_list;
}

void
revmove_from_free_list(struct s_block **free_list, struct s_block *block)
{
	if (*free_list == block)
	{
		*free_list = block->n_free_list;
	}

	struct s_block *ptr;

	ptr = *free_list;
	while (ptr != NULL)
	{
		if (ptr->n_free_list == block)
		{
			ptr->n_free_list = block->n_free_list;
		}
		ptr = ptr->n_free_list;
	}
	return ;
}

struct s_area *
find_area(void *block, size_t *zone_id)
{
	for (size_t i = 0; i < 3; i++)
	{
		struct s_area *area;

		area = heap.area[i];

		while (area != NULL)
		{
			if ((unsigned char *)area < (unsigned char *)block && (unsigned char *)block < ((unsigned char *)area) + area->size)
			{
				*zone_id = i;
				return area;
			}
			area = area->next;
		}
	}
	*zone_id = (size_t)-1;
	return NULL;
}

struct s_block *
find_block_in_area(void *ptr, struct s_area *area)
{
	struct s_block *block;

	block = area->data_ptr;
	while (block->prev_block != NULL || block == area->data_ptr)
	{
		if (&block->data == (unsigned char *)ptr)
		{
			return block;
		}
		block = (struct s_block *)(&block->data + block->size);
	}
	return NULL;
}