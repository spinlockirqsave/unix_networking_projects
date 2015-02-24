CC		= gcc
CFLAGS		= -c -g -Wall -D_GNU_SOURCE -std=gnu99 #suppress main.c:8: error: ‘for’ loop initial declarations are only allowed in C99 mode
LDFLAGS		= #-lpcap
SOURCES		= main.c
INCLUDES	= -I.
OBJECTS		= $(SOURCES:.c=.o)
TARGET		= prog

all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -rf $(OBJECTS) $(TARGET)
