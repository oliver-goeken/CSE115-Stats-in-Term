# sourced pretty much in its entirety from https://www.reddit.com/r/cpp_questions/comments/zmm6ur/help_to_use_a_separate_directory_for_objects_with/

OUT = stats

SRCDIR = src/
INCDIR = include/
OBJDIR = .obj/
TESTDIR = tests/
TESTOBJDIR = $(TESTDIR).obj/

CC = cc
LDLIBS = -lncurses
CFLAGS = -g -Wall -Wextra -I$(INCDIR)

CFILES = $(wildcard $(SRCDIR)*.c)
OBJS := $(patsubst $(SRCDIR)%.c,$(OBJDIR)%.o,$(CFILES))

TESTS = $(wildcard $(TESTDIR)*.c)
TEST_OUTS := $(patsubst $(TESTDIR)test_%.c,$(TESTDIR)test_%,$(TESTS))
TEST_OBJS := $(patsubst $(TESTDIR)%.c,$(TESTOBJDIR)%.o,$(TESTS))
TEST_FLAGS = -I$(TESTDIR)


all: $(OUT)

.PHONY: test
test: $(TEST_OUTS)


$(OUT): $(OBJS)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJS) $(LDLIBS)

$(TEST_OUTS): $(TEST_OBJS)
	$(CC) $(CFLAGS) $(TEST_FLAGS) -o $@ $(TEST_OBJS) $(LDLIBS)

$(TESTOBJDIR)%.o: $(TESTDIR)%.c
	$(CC) $(CFLAGS) $(TEST_FLAGS) -c -o $@ $<

$(OBJDIR)%.o: $(SRCDIR)%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TESTDIR)%.c:;

$(OBJDIR):
	mkdir -p $@

$(TESTOBJDIR):
	mkdir -p $@


.PHONY: clean
clean:
	rm -f $(OBJDIR)*.o $(OUT)
	rm -f $(TEST_OUTS)
