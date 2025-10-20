# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: rivoinfo <rivoinfo@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/03/22 08:39:35 by rhanitra          #+#    #+#              #
#    Updated: 2025/10/07 15:16:48 by rivoinfo         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

UTILS = ftToInt.cpp ftReadFile.cpp ftSplitStr.cpp ftStrdup.cpp \
	extractFileContent.cpp extractFilename.cpp splitParts.cpp \
	appendToCSV.cpp extractFieldName.cpp httpUtils.cpp
		
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

CXX = c++

SRC = $(UTILS)

OBJ_DIR = ./obj

OBJ = $(addprefix $(OBJ_DIR)/,$(notdir $(SRC:.cpp=.o)))

RM = rm -rf

AR = ar rcs

NAME = libutils.a


all : $(NAME)

$(NAME) : $(OBJ)
	$(AR) $(NAME) $^

$(OBJ_DIR)/%.o : %.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $< 


$(OBJ_DIR) :
	mkdir -p $(OBJ_DIR)

clean :
	$(RM) $(OBJ_DIR)
	$(RM) $(OBJ)
	
fclean : clean
	$(RM) $(NAME)

re : fclean all

.PHONY: all clean fclean re bonus
