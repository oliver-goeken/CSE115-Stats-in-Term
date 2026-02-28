# sourced partially from https://www.reddit.com/r/cpp_questions/comments/zmm6ur/help_to_use_a_separate_directory_for_objects_with/

OUT = stats

SRCDIR = src/
INCDIR = include/
LIBDIR = lib/
OBJDIR = .obj/
TESTDIR = tests/
TESTOBJDIR = $(TESTDIR).obj/

CC = cc
LDLIBS = -lncurses -lpthread -ldl
CFLAGS = -g -Wall -Wextra -I$(INCDIR) -I$(LIBDIR)

CFILES = $(wildcard $(SRCDIR)*.c)
OBJS := $(patsubst $(SRCDIR)%.c,$(OBJDIR)%.o,$(CFILES))

LIBCFILES = lib/cJSON.c lib/sqlite3.c
LIBOBJS := $(patsubst $(LIBDIR)%.c,$(OBJDIR)%.o,$(LIBCFILES))


TESTS = $(wildcard $(TESTDIR)test_*.c)
TEST_C_FILES = $(wildcard $(TESTDIR)*.c)
TEST_OUTS := $(patsubst $(TESTDIR)%.c,$(TESTDIR)%,$(TESTS))
TEST_OBJS := $(patsubst $(TESTDIR)%.c,$(TESTOBJDIR)%.o,$(TEST_C_FILES))
TEST_FLAGS = -I$(TESTDIR) -I$(SRCDIR)


all: $(OUT)

.PHONY: test
test: $(TEST_OUTS)


$(OUT): $(OBJS) $(LIBOBJS)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJS) $(LIBOBJS) $(LDLIBS)

#clunky solution; should really be better
$(TEST_OUTS): $(TEST_OBJS) $(OBJS)
	$(CC) $(CFLAGS) $(TEST_FLAGS) -o $@ $(TEST_OBJS) $(patsubst .obj/stats.o,,$(OBJS)) $(LDLIBS)

$(TESTOBJDIR)%.o: $(TESTDIR)%.c
	mkdir -p $(TESTOBJDIR)
	$(CC) $(CFLAGS) $(TEST_FLAGS) -c -o $@ $<

$(OBJDIR)%.o: $(SRCDIR)%.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)%.o: $(LIBDIR)%.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TESTDIR)%.c:;

.PHONY: clean
clean:
	rm -f $(OBJDIR)*.o $(OUT) $(OBJDIR)*.o
	rm -f $(TESTOBJDIR)*.o
	rm -f $(TEST_OUTS)
