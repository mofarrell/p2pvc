CC=gcc
OBJDIR=objs
SRCDIR=src
INCDIR=$(SRCDIR)/inc
CFLAGS+=-I$(INCDIR)

SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

CFLAGS+=-O2 -Wall -std=c99
CFLAGS_DEBUG+=-O0 -g3 -Werror -DDEBUG -pedantic
LDFLAGS+=-lxcb -lxcb-xkb -lxcb-xinerama -lxcb-randr -lcairo -lpthread

all: p2pvc

debug: CC+=$(CFLAGS_DEBUG)
debug: p2pvc .FORCE

.FORCE:

p2pvc: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

vc: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

audio: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJS): | $(OBJDIR)
$(OBJDIR):
	mkdir -p $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(wildcard $(INCDIR)/*.h) Makefile
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -rf $(OBJDIR)

