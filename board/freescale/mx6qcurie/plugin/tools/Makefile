CC = gcc
CFLAGS = -O2

OBJS = data_gen.o crc32.o

data_gen.elf: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	rm -rf *.o *.elf

