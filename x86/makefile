CC=gcc

CFLAGS= -g -O0


LDFLAGS=

SOURCES=main.c \
	lwe.c \
	lwe_original.c
		
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
	$(RM) $(EXECUTABLE) *.o perf1.dsu


