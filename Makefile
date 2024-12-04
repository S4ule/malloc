
SRCDIR	=	./srcs
SRCS	=	malloc.c\
			free.c\
			show_alloc_mem.c\
			alloc_utils.c

OBJDIR	=	./objs
OBJS	=	$(addprefix $(OBJDIR)/,$(SRCS:.c=.o))

ifeq ($(HOSTTYPE),)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

LIBS    =   
INCLUDE =	-I ./include
CFLAGS	=	-g3 -Wall -Wextra -Werror

NAME	=	libft_malloc_$(HOSTTYPE).a

default:
	make all

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	gcc $(CFLAGS) $(INCLUDE) $(LIBS) -c $< -o $@

$(OBJDIR):
	mkdir $(OBJDIR)

$(NAME):	$(OBJDIR) $(OBJS)
	ar rc $(NAME) $(OBJS)
	ranlib $(NAME)

all:	$(NAME)
	make test

clean:
	rm -f $(OBJS)
	rm -Rf $(OBJDIR)

fclean:		clean
	rm -f $(NAME)
	rm -f ./a.out

re:
	make fclean
	make all

test:
	gcc -g3 main.c -L. -lft_malloc_x86_64_Linux -I include

.PHONY:	default all clean fclean re test
