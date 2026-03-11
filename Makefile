# sourced partially from https://www.reddit.com/r/cpp_questions/comments/zmm6ur/help_to_use_a_separate_directory_for_objects_with/

OUT = stats

SRCDIR = src/
INCDIR = include/
LIBDIR = lib/
OBJDIR = .obj/
TESTDIR = tests/
TESTOBJDIR = $(TESTDIR).obj/

CC = clang
LDLIBS = -lncurses
CFLAGS = -g -Wall -Wextra -I$(INCDIR) -I$(LIBDIR)

CFILES = $(wildcard $(SRCDIR)*.c)
OBJS := $(patsubst $(SRCDIR)%.c,$(OBJDIR)%.o,$(CFILES))

LIBCFILES = lib/cJSON.c lib/sqlite3.c
LIBOBJS := $(patsubst $(LIBDIR)%.c,$(OBJDIR)%.l,$(LIBCFILES))


TESTS = $(wildcard $(TESTDIR)test_*.c)
TEST_OUTS := $(patsubst $(TESTDIR)%.c,$(TESTDIR)%,$(TESTS))
TEST_FLAGS = -I$(TESTDIR) -I$(SRCDIR)
TEST_PROJECT_OBJS := $(filter-out $(OBJDIR)stats.o $(OBJDIR)panel.o,$(OBJS))


all: $(OUT)

.PHONY: test
test: $(TEST_OUTS)


$(OUT): $(OBJS) $(LIBOBJS)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJS) $(LIBOBJS) $(LDLIBS)

$(TESTDIR)test_%: $(TESTDIR)test_%.c $(TESTDIR)support.c $(TESTDIR)unity.c $(TEST_PROJECT_OBJS) $(LIBOBJS)
	$(CC) $(CFLAGS) $(TEST_FLAGS) -o $@ $< $(TESTDIR)support.c $(TESTDIR)unity.c $(TEST_PROJECT_OBJS) $(LIBOBJS) $(LDLIBS)

$(OBJDIR)%.o: $(SRCDIR)%.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)%.l: $(LIBDIR)%.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TESTDIR)%.c:;

.PHONY: full-clean
full-clean: clean
	rm -f stats.log
	rm -f spotifyHistory.db

.PHONY: clean
clean:
	rm -f $(OBJDIR)*.o $(OUT) $(OBJDIR)*.d
	rm -f $(TEST_OUTS)
