NAME        = ircbot
CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98
SRCS_DIR    = srcs
OBJS_DIR    = objs
INC_DIR     = includes
SHARED_SRC  = ../src
SHARED_INC  = ../include

SRCS_FILES  = Bot.cpp Parser.cpp

OBJS        = $(addprefix $(OBJS_DIR)/, $(SRCS_FILES:.cpp=.o))
IFLAGS      = -I$(INC_DIR) -I$(SHARED_INC)

vpath %.cpp $(SRCS_DIR) $(SHARED_SRC)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJS_DIR)/%.o: %.cpp
	mkdir -p $(OBJS_DIR)
	$(CXX) $(CXXFLAGS) $(IFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re