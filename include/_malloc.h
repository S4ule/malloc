#ifndef ___MALLOC_H__
# define ___MALLOC_H__

# include <sys/mman.h>
	// void *mmap()
	// int munmap()

# include <stddef.h>
	// NULL

# include <unistd.h>
	// sysconf()

# ifndef TINY_ALLOC
#  define TINY_ALLOC 128
# elif TINY_ALLOC < 1
#  error "TINY_ALLOC < 1"
# endif

# ifndef SMALL_ALLOC
#  define SMALL_ALLOC 2048
# elif SMALL_ALLOC <= TINY_ALLOC
#  error "SMALL_ALLOC <= TINY_ALLOC"
# endif

# ifndef LARGE_ALLOC
#  define LARGE_ALLOC 8112
# elif LARGE_ALLOC <= SMALL_ALLOC
#  error "LARGE_ALLOC <= SMALL_ALLOC"
# endif

# ifndef N_BLOCK_IN_AREA
#  define N_BLOCK_IN_AREA 100
# elif N_BLOCK_IN_AREA < 1
#  error  "N_BLOCK_IN_AREA < 1"
# endif

# ifndef M_ALIGN
#  define M_ALIGN _Alignof(long double)
# elif M_ALIGN < 1
#  error "M_ALIGN is < 1"
# endif

struct s_block
{
	struct s_block	*prev_block;	// PTR to prev block (in use or not)
	size_t			size;			// SIZE of the current block (DATA + PADING)
	struct s_block	*n_free_list;	// PTR to the next free_block in free_list or NULL if the block is in use (that allow for better caching re-use of the heap)
	unsigned char	data;			// DATA + PADING, Used to init the data area
}	__attribute__((packed, aligned(M_ALIGN)));

# define BLOCK_INFO_SIZE (sizeof(struct s_block	*) * 2 + sizeof(size_t))

struct s_area
{
	size_t				size;				// SIZE of the AERA
	struct s_area		*next;				// Next AREA
	struct s_block		*data_ptr;			// PTR to the first BLOCK of the AREA
	unsigned char		data;				// Used to init the data area
}	__attribute__((packed, aligned(M_ALIGN)));

# define AREA_INFO_SIZE (sizeof(size_t) + sizeof(struct s_area *) + sizeof(struct s_block *))

struct s_heap
{
	struct s_area	*area[3];
	struct s_block	*free_list[3];

	size_t			n_alloced_block;

//	struct s_area *ultra_large_area;
	struct s_block	*free_list_end;
};

extern struct s_heap heap;

void *ft_memcpy(void *dest, const void *src, size_t n);
struct s_block *update_free_list(struct s_block *free_list, struct s_block *new_block);
void revmove_from_free_list(struct s_block **free_list, struct s_block *block);
struct s_area *find_area(void *block, size_t *zone_id);
struct s_block *find_block_in_area(void *ptr, struct s_area *area);
size_t get_padding_size(void *addr);

#endif /* ___MALLOC_H__ */