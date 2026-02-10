NAME := ircserv

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -MMD -MP -g

INCLUDES := -Iincludes

SRCS := \
    main.cpp \
    srcs/AServer.cpp \
    srcs/Channel.cpp \
    srcs/ChannelManagement.cpp \
    srcs/ChannelRegistry.cpp \
    srcs/Client.cpp \
    srcs/ClientManagement.cpp \
    srcs/ClientRegistry.cpp \
    srcs/Command.cpp \
    srcs/CommandFactory.cpp \
    srcs/Server.cpp \
    srcs/Session.cpp \
    srcs/SessionManagement.cpp \
    srcs/SessionRegistry.cpp \
    srcs/utils.cpp \
    srcs/Validator.cpp

OBJS := $(SRCS:.cpp=.o)
DEPS := $(OBJS:.o=.d)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

bonus: $(NAME)
	@make -C bot

clean:
	rm -f $(OBJS)
	rm -f $(DEPS)
	@make -C bot clean

fclean: clean
	rm -f $(NAME)
	@make -C bot fclean

re:
	$(MAKE) fclean
	$(MAKE) all

.PHONY: all clean fclean re bonus

-include $(DEPS)