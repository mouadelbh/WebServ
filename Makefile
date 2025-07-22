# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/24 10:08:38 by mel-bouh          #+#    #+#              #
#    Updated: 2025/06/20 05:29:30 by mel-bouh         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CC = c++
FLAGS = -Wall -Wextra -Werror #-std=c++98 # Added -std=c++98 as it's common for such projects, adjust if needed

NAME = webserv

SRCS = src/main.cpp src/Client.cpp src/Server.cpp src/Request.cpp \
	src/Response.cpp src/utils.cpp src/utils2.cpp src/RequestParser.cpp \
	src/ServerManager.cpp Parse_Config/Parser.cpp

OBJS = $(SRCS:.cpp=.o)

HEADER_FILES = includes/Client.hpp includes/Server.hpp includes/include.hpp includes/ServerManager.hpp Parse_Config/Parser.hpp

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(FLAGS) -o $(NAME) $(OBJS)
	@echo "WebServ compiled successfully!"
	@echo "Usage: ./$(NAME) [config_file]"

%.o: %.cpp $(HEADER_FILES)
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
