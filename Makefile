NAME := ircserv

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98

INCLUDES := -Iincludes

SRCS := \
	main.cpp \
	srcs/Server.cpp \
	srcs/utils.cpp \
	srcs/Channel.cpp \
	srcs/Manager.cpp


OBJS := $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
