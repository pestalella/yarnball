WRKDIR = `pwd`

CC = gcc
CXX = g++
LD = g++

INC = 
LIBDIR = 
LIB = -lgif -lglut -lGLU -lGL -lIL -lILU -lpng -lm
LDFLAGS =
#LDFLAGS = -pg -g3

CFLAGS = -Wall -W -O2
#CFLAGS = -g3
OBJDIR = build


SRCDIR = src
OUT_BINARY = $(OBJDIR)/yarnball

SRC=main.cpp Point.cpp sphere.cpp YarnBall.cpp AnimatedGifSaver.cpp
_OBJ := $(addsuffix .o,$(basename $(SRC)))
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

all: $(OUT_BINARY)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

before_linking: 
	test -d $(OBJDIR) || mkdir -p $(OBJDIR)

release: before_linking yarnball 

$(OUT_BINARY): before_linking $(OBJS) 
	$(LD) $(LIBDIR) -o $@ $(OBJS) $(LDFLAGS) $(LIB)

clean: 
	rm -f $(OBJS) $(OUT_BINARY)
	rm -rf $(OBJDIR)

.PHONY: before_linking clean
