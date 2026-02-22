# sourced partially from https://www.reddit.com/r/cpp_questions/comments/zmm6ur/help_to_use_a_separate_directory_for_objects_with/

OUT = stats


SRCDIR = src/
INCDIR = include/
OBJDIR = .obj/
TESTDIR = tests/
TESTOBJDIR = $(TESTDIR).obj/

CC = cc
LDLIBS = -lncurses

CFILES = $(wildcard $(SRCDIR)*.c)
OBJS := $(patsubst $(SRCDIR)%.c,$(OBJDIR)%.o,$(CFILES))


CPP_SRCDIR = src_cpp/
CPP_INCDIR = include_cpp/
CPP_OBJDIR = .obj_cpp/

CFLAGS = -g -Wall -Wextra -std=c++17 -I$(CPP_INCDIR) -I$(INCDIR)

CXX = g++

CPPFILES = $(wildcard $(CPP_SRCDIR)*.cpp)
CPP_OBJS := $(patsubst $(CPP_SRCDIR)%.cpp,$(CPP_OBJDIR)%.o,$(CPPFILES))


TESTS = $(wildcard $(TESTDIR)test_*.c)
TEST_C_FILES = $(wildcard $(TESTDIR)*.c)
TEST_OUTS := $(patsubst $(TESTDIR)%.c,$(TESTDIR)%,$(TESTS))
TEST_OBJS := $(patsubst $(TESTDIR)%.c,$(TESTOBJDIR)%.o,$(TEST_C_FILES))
TEST_FLAGS = -I$(TESTDIR) -I$(SRCDIR)


all: $(OUT)

.PHONY: test
test: $(TEST_OUTS)


$(OUT): $(OBJS) $(CPP_OBJS)
	$(CXX) $(CFLAGS) -o $(OUT) $(OBJS) $(CPP_OBJS) $(LDLIBS)

#clunky solution; should really be better
$(TEST_OUTS): $(TEST_OBJS) $(OBJS)
	$(CC) $(CFLAGS) $(TEST_FLAGS) -o $@ $(TEST_OBJS) $(patsubst .obj/stats.o,,$(OBJS)) $(LDLIBS)

$(TESTOBJDIR)%.o: $(TESTDIR)%.c
	mkdir -p $(TESTOBJDIR)
	$(CC) $(CFLAGS) $(TEST_FLAGS) -c -o $@ $<

$(OBJDIR)%.o: $(SRCDIR)%.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(CPP_OBJDIR)%.o: $(CPP_SRCDIR)%.cpp
	mkdir -p $(CPP_OBJDIR)
	$(CXX) $(CFLAGS) -c -o $@ $<

$(TESTDIR)%.c:;

.PHONY: clean
clean:
	rm -f $(OBJDIR)*.o $(OUT)
	rm -f $(TESTOBJDIR)*.o
	rm -f $(TEST_OUTS)
