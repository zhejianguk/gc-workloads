CC = riscv64-unknown-linux-gnu-gcc

# OFLAGS := -O3 -DRISCV -march=rv64imafd -DUSE_PUBLIC_MALLOC_WRAPPERS  -shared -fpic
OFLAGS := -O3 -DRISCV -march=rv64imafd -DUSE_PUBLIC_MALLOC_WRAPPERS
CFLAGS:= -O3 -DRISCV -march=rv64imafd
CFLAGS_CM:= -O3 -DRISCV -march=rv64imafd -Wl,--allow-multiple-definition


#CC = gcc
#CFLAGS := -O3 -std=gnu99

malloc: malloc.c
	${CC} ${OFLAGS} -c malloc.c
	
tc_pmc: tc_pmc.c
	${CC} ${CFLAGS} -o tc_pmc.riscv tc_pmc.c -lpthread

tc_sanitiser: tc_sanitiser.c
	${CC} ${CFLAGS_CM} -o tc_sanitiser.riscv tc_sanitiser.c malloc.o -lpthread

tc_shadowstack-s: tc_shadowstack-s.c
	${CC} ${CFLAGS} -o tc_shadowstack-s.riscv tc_shadowstack-s.c -lpthread

tc_shadowstack-m: tc_shadowstack-m.c
	${CC} ${CFLAGS} -o tc_shadowstack-m.riscv tc_shadowstack-m.c -lpthread

clean:
	rm -f *.o *.riscv *.so

