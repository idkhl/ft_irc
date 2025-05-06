CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -g -MMD -std=c++98

OBJS_DIR = obj

SRCS =	src/Server.cpp \
	src/main.cpp \
	src/Channel.cpp

DEPS = $(OBJS:.o=.d)

OBJS = $(SRCS:%.cpp=$(OBJS_DIR)/%.o)

NAME = ircserv

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ -g
	
$(OBJS_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

all: $(NAME)

clean:
	rm -rf $(OBJS_DIR) $(DEPS)

fclean: clean
	rm -f $(NAME)

re: fclean all

-include $(DEPS)

.PHONY: all clean fclean re