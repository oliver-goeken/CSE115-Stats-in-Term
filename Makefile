# sourced pretty much in its entirety from https://www.reddit.com/r/cpp_questions/comments/zmm6ur/help_to_use_a_separate_directory_for_objects_with/

OUT = stats

SRCDIR = src/
INCDIR = include/
OBJDIR = .obj/

CC = cc
LDLIBS = -lncurses
CFLAGS = -g -Wall -Wextra -I$(INCDIR)

CFILES = $(wildcard $(SRCDIR)*.c)
OBJS := $(patsubst $(SRCDIR)%.c,$(OBJDIR)%.o,$(CFILES))


all: $(OUT)

$(OUT): $(OBJS)
	$(CC) $(CLFAGS) -o $(OUT) $(OBJS) $(LDLIBS)

$(OBJDIR)%.o: $(SRCDIR)%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $@


.PHONY: clean
clean:
	rm -f $(OBJDIR)*.o $(OUT)
