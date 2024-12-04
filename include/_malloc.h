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
#  define TINY_ALLOC 32
# elif TINY_ALLOC < 1
#  error "TINY_ALLOC < 1"
# endif

# ifndef SMALL_ALLOC
#  define SMALL_ALLOC 128
# elif SMALL_ALLOC <= TINY_ALLOC
#  error "SMALL_ALLOC <= TINY_ALLOC"
# endif

# ifndef LARGE_ALLOC
#  define LARGE_ALLOC 512
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

# define BLOCK_HEADER_SIZE	(sizeof(struct s_block	*) + sizeof(size_t))
# define BLOCK_TAIL_SIZE	sizeof(struct s_block *)
# define BLOCK_INFO_SIZE	(BLOCK_HEADER_SIZE + BLOCK_TAIL_SIZE)

# define AREA_INFO_SIZE (sizeof(size_t) * 2 + sizeof(struct s_area *) + sizeof(unsigned char *))

# define ALLOCED_FLAG ((size_t)0x1 << (sizeof(size_t) * 8 - 1))

size_t get_block_size(size_t x);
size_t get_area_size(size_t x);
size_t get_padding_size(void *x);

/*
	area :
		n_block | area_size | next_area | end_free_list | data_ptr
		... (blocks) ...

		size_t  | size_t | struct * | NULL


	Used Block :
		data_size | DATA | begin_block_ptr | ...
		data_size | DATA | begin_block_ptr | ...

	Un-use Block :
		data_size | free_list | ... | begin_block_ptr | ...
		data_size | free_list | ... | begin_block_ptr | ...
*/

struct s_block
{
	size_t			size;
	struct s_block	*n_free_list;
	unsigned char	data;
}	__attribute__((packed, aligned(M_ALIGN)));

struct s_area
{
	size_t				n_block;
	size_t				area_size;
	struct s_area		*next_area;
	unsigned char		*null_ptr;
	struct s_block		block;
}	__attribute__((packed, aligned(M_ALIGN)));

struct s_heap
{
	struct s_area	*area[3];
	struct s_block	*free_list[3];

//	struct s_area *ultra_large_area;
};

extern struct s_heap heap;

#endif /* ___MALLOC_H__ */