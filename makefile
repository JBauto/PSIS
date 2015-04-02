CC := gcc 

LIBS := -pthread -lrt

BINDIR := bin/
OBJDIR := obj/
SCRDIR := src/

INCDIRS =

SOURCES  := $(wildcard $(SRCDIR)*.c) $(wildcard $(SRCDIR)*/*.c)
INCDIRS  += $(sort $(dir $(wildcard $(SRCDIR)*.h) $(wildcard $(SRCDIR)*/*.h)))
OBJECTS  := $(addprefix $(OBJDIR), $(SOURCES:.c=.o))


CFLAGS =  $(foreach includedir,$(INCDIRS),-I$(includedir)) -c -g -Wall

LFLAGS =  $(foreach librarydir,$(LIBDIR),-L($librarydir)) $(LIBS)

all: server.out

server.out: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LFLAGS)

$(OBJDIR)%.o: $(SRCDIR)%.c
	$(CC) -o $@ $< $(CFLAGS)


clean:
	rm obj/src/*
