# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: safandri <safandri@student.42antananari    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/28 14:08:13 by rhanitra          #+#    #+#              #
#    Updated: 2025/09/20 14:59:38 by safandri         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g -I $(DIR_UTILS)
RM = rm -rf

DIR_WEBSERV = ./srcs
DIR_UTILS = ./utils
OBJ_DIR = ./obj
INC_DIR = ./include

WEBSERV = ../main.cpp Client.cpp Server.cpp HTTPRequest.cpp HTTPResponse.cpp ServerManager.cpp CGIHandler.cpp
#  ErrorHandler.cpp ConfigParser.cpp

SRC_WEBSERV = $(addprefix $(DIR_WEBSERV)/,$(WEBSERV))
UTILS = $(DIR_UTILS)/libutils.a

OBJ_WEBSERV = $(addprefix $(OBJ_DIR)/,$(notdir $(SRC_WEBSERV:.cpp=.o)))

all: $(NAME)

$(OBJ_DIR)/%.o: $(DIR_WEBSERV)/%.cpp | $(OBJ_DIR)
	@$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(UTILS):
	@$(MAKE) -C $(DIR_UTILS)

$(NAME): $(OBJ_WEBSERV) $(UTILS)
	@$(CXX) $(CXXFLAGS) -o $@ $(OBJ_WEBSERV) $(UTILS)

clean:
	@$(RM) $(OBJ_DIR) vgcore.*
	@$(MAKE) -C $(DIR_UTILS) clean
	@clear

fclean: clean
	@$(RM) $(NAME)
	@$(MAKE) -C $(DIR_UTILS) fclean

re: fclean all

run: all
	@clear
	@./$(NAME)

runre: re
	@./$(NAME)

debug: all
	@valgrind --track-origins=yes --leak-check=full ./$(NAME)

# debug: all
# 	@valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./$(NAME)

.PHONY: all clean fclean re run runre