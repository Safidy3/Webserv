# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: rivoinfo <rivoinfo@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/28 14:08:13 by rhanitra          #+#    #+#              #
#    Updated: 2025/09/24 16:36:28 by rivoinfo         ###   ########.fr        #
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

WEBSERV = configParser.cpp requestParser.cpp server.cpp responseBuilder.cpp \
		handleCGI.cpp handleErrors.cpp \
          ../main.cpp

SRC_WEBSERV = $(addprefix $(DIR_WEBSERV)/,$(WEBSERV))
UTILS = $(DIR_UTILS)/libutils.a


OBJ_WEBSERV = $(addprefix $(OBJ_DIR)/,$(notdir $(SRC_WEBSERV:.cpp=.o)))

all: $(NAME)

$(OBJ_DIR)/%.o: $(DIR_WEBSERV)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(UTILS):
	$(MAKE) -C $(DIR_UTILS)

$(NAME): $(OBJ_WEBSERV) $(UTILS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_WEBSERV) $(UTILS)

watch:
	./bootstrap/dart-sass/sass ./bootstrap/sass/style.scss ./bootstrap/sass/style.css --watch

compress:
	./bootstrap/dart-sass/sass ./bootstrap/sass/style.scss ./bootstrap/sass/style.css --style=compressed --no-source-map
	
clean:
	$(RM) $(OBJ_DIR)
	$(MAKE) -C $(DIR_UTILS) clean

fclean: clean
	$(RM) $(NAME)
	$(MAKE) -C $(DIR_UTILS) fclean

re: fclean all

.PHONY: all clean fclean re

