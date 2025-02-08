# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mmaila <mmaila@student.42.fr>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/11/21 09:47:15 by nazouz            #+#    #+#              #
#    Updated: 2025/02/07 15:14:43 by mmaila           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME			= 		webserv

CPP				= 		g++

CPPFLAGS		= 		-Wall -Werror -Wextra -fsanitize=address -g -std=c++98

INCLUDE			=		\
						./_Config/Config.hpp \
						./IEventHandler.hpp \
						./Client/Request/Request.hpp \
						./Client/Response/Response.hpp \
						./Client/Response/Error.hpp \
						./Server/ServerHandler.hpp \
						./Client/ClientHandler.hpp \
						./HTTPServer/Webserv.hpp \
						./Client/CGI/CGIHandler.hpp \


SRCS			= 		\
						./_Config/Config.cpp \
						./_Config/Parsing.cpp \
						./_Config/ServerConstructor.cpp \
						./_Config/Validators.cpp \
						./HTTPServer/Webserv.cpp \
						./Client/Request/Request.cpp \
						./Client/Request/_ControlCenter.cpp \
						./Client/Request/Headers.cpp \
						./Client/Request/Body.cpp \
						./Client/Request/RequestData.cpp \
						./Client/Response/Response.cpp \
						./Client/Response/Error.cpp \
						./Client/Response/Read.cpp \
						./Client/Response/Headers.cpp \
						./Client/CGI/CGIHandler.cpp \
						./Client/ClientHandler.cpp \
						./Client/Helpers.cpp \
						./Server/ServerHandler.cpp \
						./Utils/Helpers.cpp \
						./main.cpp

OBJDIR			=		./objects

OBJS			= 		$(SRCS:%.cpp=$(OBJDIR)/%.o)

all : $(NAME)

$(OBJDIR)/%.o : %.cpp $(INCLUDE)
	@mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) -c $< -o $@

$(NAME) : $(OBJS)
	$(CPP) $(CPPFLAGS) $(OBJS) -o $(NAME)

clean :
	rm -rf $(OBJDIR)

fclean : clean
	rm -rf $(NAME)

re : fclean all