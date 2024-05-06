CC=gcc
CFLAGS=-Wall
PTHREAD=-lpthread

# Define all targets
TARGETS=copy ForkCopy PipeCopy Shell MergesortSingle MergesortMulti

all: $(TARGETS)

copy: copy.c
	$(CC) $(CFLAGS) copy.c -o copy

ForkCopy: ForkCopy.c
	$(CC) $(CFLAGS) ForkCopy.c -o ForkCopy

PipeCopy: PipeCopy.c
	$(CC) $(CFLAGS) PipeCopy.c -o PipeCopy

Shell: Shell.c
	$(CC) $(CFLAGS) Shell.c -o Shell

MergesortSingle: MergesortSingle.c
	$(CC) $(CFLAGS) MergesortSingle.c -o MergesortSingle

MergesortMulti: MergesortMulti.c
	$(CC) $(CFLAGS) $(PTHREAD) MergesortMulti.c -o MergesortMulti

clean:
	rm -f $(TARGETS)
