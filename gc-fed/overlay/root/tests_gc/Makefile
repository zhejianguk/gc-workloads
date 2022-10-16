CC = riscv64-unknown-linux-gnu-gcc

OFLAGS := -O3 -DRISCV -march=rv64imafd -DUSE_PUBLIC_MALLOC_WRAPPERS  -shared -fpic
CFLAGS:= -O3 -DRISCV -march=rv64imafd

#CC = gcc
#CFLAGS := -O3 -std=gnu99

malloc: malloc.c
	${CC} ${OFLAGS} -o malloc.so malloc.c 

tc_pmc: tc_pmc.c
	${CC} ${CFLAGS} -o tc_pmc.riscv tc_pmc.c -lpthread

tc_sanitiser: tc_sanitiser.c
	${CC} ${CFLAGS} -o tc_sanitiser.riscv tc_sanitiser.c malloc.so -lpthread

clean:
	rm -f *.o *.riscv *.so

