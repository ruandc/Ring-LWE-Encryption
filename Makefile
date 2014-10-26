CC=gcc

CFLAGS= -g -O0  -Wall


LDFLAGS=

#SOURCES=LWE2.c

SOURCES=main.c \
		unit_test.c \
		lwe.c \
		files.c
		
OBJECTS1=$(SOURCES:.S=.o)
OBJECTS2=$(OBJECTS1:.s=.o)
OBJECTS=$(OBJECTS2:.c=.o)
EXECUTABLE=LWE

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

disassemble:
	$(OBJDUMP) -S --disassemble -D $(EXECUTABLE) > $(EXECUTABLE).dump

clean:
	$(RM) $(EXECUTABLE) *.o


