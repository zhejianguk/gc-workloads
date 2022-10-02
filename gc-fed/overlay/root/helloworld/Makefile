CC = riscv64-unknown-linux-gnu-gcc
CFLAGS := -O3 -static -DRISCV -march=rv64imafd

#CC = gcc
#CFLAGS := -O3 -std=gnu99

helloworld: helloworld.c
	${CC} ${CFLAGS} -o helloworld.riscv helloworld.c

clean:
	rm -f helloworld.riscv
