CC=gcc
CFLAGS=-c -Wall -O2 -fomit-frame-pointer -frename-registers `sdl-config --cflags`
LDFLAGS=`sdl-config --libs`
SOURCES=credits.c game.c main.c select.c title.c options.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=SlugFest97DX

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

