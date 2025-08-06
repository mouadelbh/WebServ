# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/24 10:08:38 by mel-bouh          #+#    #+#              #
#    Updated: 2025/08/05 13:38:52 by mel-bouh         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Compiler and flags
CC = c++
FLAGS = -Wall -Wextra -Werror #-std=c++98 # Added -std=c++98 as it's common for such projects, adjust if needed

# Executable name
NAME = webserv

# Source files: Added Server.cpp
SRCS = src/main.cpp src/Client.cpp src/Server.cpp src/Request.cpp \
	src/Response.cpp src/utils.cpp src/utils2.cpp src/RequestParser.cpp \
	src/PostMethod.cpp Parse_Config/Parser.cpp

# Object files (derived from SRCS)
OBJS = $(SRCS:.cpp=.o)

# Header files (dependencies for object files)
# Ensure all necessary headers are listed if they influence compilation of multiple .cpp files
# For simplicity, we can make all .o files depend on all .hpp files,
# or be more precise if needed.
HEADER_FILES = includes/Client.hpp includes/Server.hpp includes/include.hpp Parse_Config/Parser.hpp

# Default target
all: $(NAME)

# Rule to link the executable
$(NAME): $(OBJS)
	$(CC) $(FLAGS) -o $(NAME) $(OBJS)
	./webserv

# Pattern rule to compile .cpp files into .o files
# This rule says: to make a .o file, find the corresponding .cpp file
# and compile it. It also depends on the listed header files.
%.o: %.cpp $(HEADER_FILES)
	$(CC) $(FLAGS) -c $< -o $@

# Rule to clean object files
clean:
	rm -f $(OBJS)

# Rule to clean object files and the executable
fclean: clean
	rm -f $(NAME)

# Rule to recompile everything
re: fclean all

# Phony targets (targets that don't represent actual files)
.PHONY: all clean fclean re
