CC = riscv64-unknown-linux-gnu-gcc

OFLAGS := -O3 -DRISCV -march=rv64imafd -DUSE_PUBLIC_MALLOC_WRAPPERS
CFLAGS_nomalloc := -O3 -static -DRISCV -march=rv64imafd
CFLAGS := -O3 -static -DRISCV -march=rv64imafd -Wl,--allow-multiple-definition -DUSE_PUBLIC_MALLOC_WRAPPERS
CFLAGS_SL := -O3 -DRISCV -march=rv64imafd -Wl,--allow-multiple-definition -DUSE_PUBLIC_MALLOC_WRAPPERS

#CC = gcc
#CFLAGS := -O3 -std=gnu99

malloc: malloc.c
	${CC} ${OFLAGS} -c malloc.c

tc_pmc: tc_pmc.c
	${CC} ${CFLAGS_nomalloc} -o tc_pmc.riscv tc_pmc.c -lpthread

tc_sanitiser: tc_sanitiser.c
	${CC} ${CFLAGS_SL} -shared -o tc_sanitiser.riscv tc_sanitiser.c malloc.o -lpthread

clean:
	rm -f *.o *.riscv 
