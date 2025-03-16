CC = cc
NAME = server
CFLAGS = -Wall -Wextra -Werror -fsanitize=address
INCS = incs
SRCS = $(wildcard srcs/*.c)
OBJS = $(SRCS:.c=.o)

# 컴파일 규칙
.c.o :
	$(CC) -c $(CFLAGS) -o $@ $^ -I $(INCS)

all : $(NAME)

$(NAME) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

clean :
	$(RM) $(OBJS)

fclean : clean
	$(RM) $(NAME)

re :
	make fclean
	make all

only :
	make re
	make clean

.PHONY : all clean fclean re .c.o