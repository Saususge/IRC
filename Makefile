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
	srcs/Server.cpp \
	srcs/Session.cpp \
	srcs/SessionManagement.cpp \
	srcs/SessionRegistry.cpp \
	srcs/utils.cpp \
	srcs/Validator.cpp \
	srcs/numeric.cpp \
	srcs/command/CommandUtility.cpp \
	srcs/command/CommandContext.cpp \
	srcs/command/PassCommand.cpp \
	srcs/command/NickCommand.cpp \
	srcs/command/UserCommand.cpp \
	srcs/command/QuitCommand.cpp \
	srcs/command/NamesCommand.cpp \
	srcs/command/TopicCommand.cpp \
	srcs/command/JoinCommand.cpp \
	srcs/command/PartCommand.cpp \
	srcs/command/KickCommand.cpp \
	srcs/command/InviteCommand.cpp \
	srcs/command/ChannelModeCommand.cpp \
	srcs/command/PrivmsgCommand.cpp \
	srcs/command/NoticeCommand.cpp \
	srcs/command/PingCommand.cpp \
	srcs/command/CommandFactory.cpp


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
	$(MAKE)

.PHONY: all clean fclean re bonus

-include $(DEPS)
