##
## EPITECH PROJECT, 2024
## B-NWP-400-MAR-4-1-myftp-selim.bouasker
## File description:
## Makefile
##

SRC	=	ftp.c \
		handle_clients.c \
		commands.c \
		check_command.c

OBJ	=	$(SRC:.c=.o)

NAME	= myftp

all:	$(NAME)

$(NAME):	$(OBJ)
	gcc $(OBJ) -o $(NAME)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re:	fclean all
