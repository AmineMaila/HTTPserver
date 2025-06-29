# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mmaila <mmaila@student.42.fr>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/11/21 09:47:15 by nazouz            #+#    #+#              #
#    Updated: 2025/03/08 19:31:25 by mmaila           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME			= 		webserv

CPP				= 		c++

CPPFLAGS		= 		-Wall -Werror -Wextra

INCLUDE			=		\
						./Config/Config.hpp \
						./IEventHandler.hpp \
						./Client/Request/Request.hpp \
						./Client/Response/AResponse.hpp \
						./Client/Response/Response.hpp \
						./Client/Response/Error.hpp \
						./Server/ServerHandler.hpp \
						./Client/ClientHandler.hpp \
						./HTTPServer/Webserv.hpp \
						./Client/CGI/CGIHandler.hpp \
						./Utils/Helpers.hpp \


SRCS			= 		\
						./Config/Config.cpp \
						./Config/Parsing.cpp \
						./Config/ServerConstructor.cpp \
						./Config/Validators.cpp \
						./HTTPServer/Webserv.cpp \
						./Client/Request/Request.cpp \
						./Client/Request/Headers.cpp \
						./Client/Request/Body.cpp \
						./Client/Response/Response.cpp \
						./Client/Response/Range.cpp \
						./Client/Response/Error.cpp \
						./Client/Response/Read.cpp \
						./Client/Response/Headers.cpp \
						./Client/CGI/CGIHandler.cpp \
						./Client/CGI/CGIHeaders.cpp \
						./Client/CGI/CGIInit.cpp \
						./Client/ClientHandler.cpp \
						./Client/URI.cpp \
						./Server/ServerHandler.cpp \
						./Utils/Helpers.cpp \
						./main.cpp

OBJDIR			=		./objects

OBJS			= 		$(SRCS:%.cpp=$(OBJDIR)/%.o)

all : $(NAME)
	@mkdir -p ~/webserv/

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
