
#include "_malloc.h"

size_t
get_block_size(size_t x)
{
	return BLOCK_INFO_SIZE + x;
}

size_t
get_area_size(size_t x)
{
	return AREA_INFO_SIZE + (get_block_size(x) + M_ALIGN - 1) * N_BLOCK_IN_AREA;
}

size_t
get_padding_size(void *x)
{
	return M_ALIGN - (unsigned long long)x % M_ALIGN;
}