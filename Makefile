SHELL	=	/bin/bash
CC	=	g++ -std=c++17 -fdiagnostics-color=always

BUILD_DIR	?= ./build
SRC_DIR		?= ./source
INCS_DIR	?= ./include
TEST_DIR	?= ./tests
GCOV_DIR	?= ./html

SRC_EXT		=	cpp

TEST_FILES	:= $(shell find $(TEST_DIR) -name *.$(SRC_EXT))
TEST_OBJ	:=	$(TEST_FILES:$(TEST_DIR)/%.$(SRC_EXT)=$(BUILD_DIR)/$(TEST_DIR)/%.o)
TEST_DEP	:=	$(TEST_OBJ:%.o=%.d)

MAIN	=	main.cpp

NAME	=	lockless_queue

SRCS	:= $(shell find $(SRC_DIR) -name *.$(SRC_EXT))
OBJS	:= $(SRCS:$(SRC_DIR)/%.$(SRC_EXT)=$(BUILD_DIR)/%.o)
DEPS	:= $(OBJS:%.o=%.d)

SRCS_T	=	$(filter-out %$(MAIN), $(SRCS))
OBJS_T	=	$(SRCS_T:$(SRC_DIR)/%.$(SRC_EXT)=$(BUILD_DIR)/%.o)

# INCS	:= $(shell find -L $(INCS_DIR) -type d)
# CFLAGS	+=	-W -Wall -Werror -Wextra $(foreach dir,$(INCS), -I $(dir))
CFLAGS	+=	-W -Wall -Werror -Wextra -I $(INCS_DIR)
ifdef DEBUG
  CFLAGS += -DDEBUG -g3
endif

LDFLAGS	+= 

YELLOW          =       \033[0;103m
RED             =       \033[0;31m
GREEN           =       \033[0;32m
NC              =       \033[0m
GREY            =       \033[90m
BLUE            =       \033[0;94m
PURPLE          =       \033[0;95m
BG_COLOR        =       \033[46m\033[30m
IRED            =       \033[0;91m

all:	$(NAME)

-include $(TEST_DEP)
-include $(DEPS)

$(NAME):	gcov_clean	$(OBJS)
	@echo -e '${NC}${BG_COLOR}Libs: $(LDFLAGS)${NC}'
	@echo -e '${BG_COLOR}Flags: $(CFLAGS)${NC}'
	@$(CC) $(OBJS) -o $(NAME) $(LDFLAGS) \
		&& echo -e '${BLUE}Create${NC}: ${YELLOW}${GREY}./$(NAME)${NC}'\
		|| (echo -e '${RED}[ FAILED ]${NC}: __Create__${GREY} ./$(NAME)${NC}' && exit 1)

tidy:
	clang-tidy $(SRCS)

debug:	 CFLAGS += -DDEBUG -g3
ifeq ($(shell objdump --syms ${NAME} 2> /dev/null | grep debug),)
  debug: fclean all
else
  debug: all
endif

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@rm -f $@
	@mkdir -p $(shell dirname $@)
	@-$(CC) -MMD -o $@ -c $< $(LDFLAGS) $(CFLAGS) \
		&& echo -e '${GREEN} [ OK ]${NC} Build $<'\
		|| echo -e '${RED}[ FAILED ]${NC} __Build__ $<'
	@echo -ne '${NC}'

$(BUILD_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp
	@rm -f $@
	@mkdir -p $(shell dirname $@)
	@-$(CC) -MMD -o $@ -c $< $(LDFLAGS) $(CFLAGS) \
		&& echo -e '${GREEN} [ OK ]${NC} Build $<'\
		|| echo -e '${RED}[ FAILED ]${NC} __Build__ $<'
	@echo -ne '${NC}'

tests_run: CFLAGS += --coverage -g3
tests_run:	$(TEST_OBJ)	$(OBJS_T)
	@$(CC) $(TEST_OBJ) $(OBJS_T) -l criterion -o crit -l gcov $(LDFLAGS) $(CFLAGS) \
		&& echo -e '${BLUE}Create${NC}: ${YELLOW}${GREY}./crit${NC}'\
		|| (echo -e '${RED}[ FAILED ]${NC}: __Create__${GREY} ./crit${NC}' && exit 1)
	openssl genrsa -out aa.key 4096
	@./crit -j1 --verbose

gcov:	clean	tests_run
	@mkdir -p $(GCOV_DIR)
	@echo -e '${BLUE}Generating coverrage into: ${NC}$(GCOV_DIR)'
	@gcovr --exclude=tests/ --html-details -s --output=$(GCOV_DIR)/coverrage.html
	@echo -e '${BLUE}Coverrage ${YELLOW}${GREY}generated${NC} !'

gcov_clean:
	@rm -f gmon.out
	@find . -name "*.gc*" -print0 -delete | sed -z -e "s/\.gc\(da\|no\)$$/\.o/" | xargs -0 rm -f
	@echo -e '${BLUE}Removed gcov files${NC} : OK'

clean: gcov_clean
	@find . -name "*~" -delete -o -name "#*#" -delete
	@rm -rf $(OBJS) $(OBJS_T) $(DEPS) $(TEST_DEP)
	@rm -rf ./build
	@rm -rf vgcore.*
	@echo -e '${BLUE}Clean${NC} : OK'

fclean:         clean
	@rm -rf crit
	@rm -rf a.out
	@rm -rf logs/
	@rm -rf $(NAME)
	@find . -name "*.d" -delete
	@echo -e '${BLUE}Fclean${NC}: ./$(NAME) removed'

re:	fclean	all

ifndef VERBOSE
MAKEFLAGS += --no-print-directory
endif

#A phony target should not be a prerequisite of a real target file;
#if it is, its recipe will be run every time make goes to update that file.
.PHONY:	all	$(NAME)	clean	fclean	re
