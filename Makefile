##
## EPITECH PROJECT, 2024
## B-NWP-400-MAR-4-1-myftp-selim.bouasker
## File description:
## Makefile
##

SRC	=	src/ftp.c \
		src/handle_clients.c \
		src/commands.c \
		src/commands_part_2.c \
		src/pasv_and_actv.c \
		src/dele_handling.c

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
