CC=gcc

CFLAGS= -g3 -O0


LDFLAGS=

SOURCES=main.c \
	lwe.c \
	lwe_original.c \
	reduction_longa_3x.c \
	luts.c \
	reduction_longa_2x.c
		
OBJECTS1=$(SOURCES:.S=.o)
OBJECTS2=$(OBJECTS1:.s=.o)
OBJECTS=$(OBJECTS2:.c=.o)
EXECUTABLE=LWE

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ ${CFLAGS}

disassemble:
	$(OBJDUMP) -S --disassemble -D $(EXECUTABLE) > $(EXECUTABLE).dump

clean:
	$(RM) $(EXECUTABLE) *.o perf1.dsu


