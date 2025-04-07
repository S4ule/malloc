
SRCDIR	=	./srcs
SRCS	=	malloc.c\
			free.c\
			realloc.c\
			alloc_utils.c\
			show_alloc_mem.c\
			ft_printf.c\

#--------------
OBJDIR		=	./objs
SRCSDIR 	=	./srcs
OUTPUTDIR	=	./outlib
#--------------
# auto
OBJS	=	$(addprefix $(OBJDIR)/,$(SRCS:.c=.o))

ifeq ($(HOSTTYPE),)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif
#--------------
LIBS    =   
INCLUDE =	-I ./include
CFLAGS	=	-g3 -Wall -Wextra -Werror

LN_NAME	=	libft_malloc.a
NAME	=	libft_malloc_$(HOSTTYPE).a
#--------------
# rules

default:
	make all

$(OBJDIR)/%.o : $(SRCSDIR)/%.c
	gcc $(CFLAGS) $(INCLUDE) $(LIBS) -c $< -o $@

$(OBJDIR):
	mkdir $(OBJDIR)
	mkdir $(OUTPUTDIR)

$(OUTPUTDIR)/$(NAME):	$(OBJDIR) $(OUTPUTDIR) $(OBJS) $(OUTPUTDIR)/malloc.h
	ar rc $(OUTPUTDIR)/$(NAME) $(OBJS)
	ranlib $(OUTPUTDIR)/$(NAME)
	ln -s $(OUTPUTDIR)/$(NAME) $(LN_NAME)

$(OUTPUTDIR)/malloc.h:
	cp ./include/malloc.h $(OUTPUTDIR)/malloc.h

$(NAME):	$(OUTPUTDIR)/$(NAME)

all:	$(OUTPUTDIR)/$(NAME)

clean:
	rm -f $(OBJS)
	rm -Rf $(OBJDIR)

fclean:		clean
	rm -Rf $(OUTPUTDIR)
	rm -f $(LN_NAME)
	rm -f ./a.out

re:
	make fclean
	make all

.PHONY:	default all clean fclean re
