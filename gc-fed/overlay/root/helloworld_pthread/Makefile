CC = riscv64-unknown-linux-gnu-gcc
CFLAGS := -O3 -static -DRISCV -march=rv64imafd

#CC = gcc
#CFLAGS := -O3 -std=gnu99

helloworld_pthread: helloworld_pthread.c
	${CC} ${CFLAGS} -o helloworld_pthread.riscv helloworld_pthread.c -lpthread

clean:
	rm -f helloworld_pthread.riscv
