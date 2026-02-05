NAME := ircserv

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -MMD -MP -g

INCLUDES := -Iincludes

SRCS := \
	main.cpp \
	srcs/AServer.cpp \
	srcs/Channel.cpp \
	srcs/ChannelRegistry.cpp \
	srcs/Client.cpp \
	srcs/ClientRegistry.cpp \
	srcs/Command.cpp \
	srcs/Server.cpp \
	srcs/Session.cpp \
	srcs/utils.cpp \


OBJS := $(SRCS:.cpp=.o)

DEPS := $(OBJS:.o=.d)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS)
	rm -f $(DEPS)

fclean: clean
	rm -f $(NAME)

re:
	$(MAKE) fclean
	$(MAKE)

.PHONY: all clean fclean re

-include $(DEPS)
