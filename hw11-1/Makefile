
BINS := collatz-list-sys collatz-ivec-sys \
        collatz-list-nu  collatz-ivec-nu  \
        collatz-list-par collatz-ivec-par

HDRS := $(wildcard *.h)
SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)

CFLAGS := -g
LDLIBS := -lpthread

all: $(BINS)

collatz-list-sys: list_main.o sys_malloc.o
	gcc $(CFLAGS) -o $@ $^ $(LDLIBS)

collatz-ivec-sys: ivec_main.o sys_malloc.o
	gcc $(CFLAGS) -o $@ $^ $(LDLIBS)

collatz-list-nu: list_main.o nu_malloc.o nu_malloc_help.o
	gcc $(CFLAGS) -o $@ $^ $(LDLIBS)

collatz-ivec-nu: ivec_main.o nu_malloc.o nu_malloc_help.o
	gcc $(CFLAGS) -o $@ $^ $(LDLIBS)

collatz-list-par: list_main.o par_malloc.o par_malloc_help.o
	gcc $(CFLAGS) -o $@ $^ $(LDLIBS)

collatz-ivec-par: ivec_main.o par_malloc.o par_malloc_help.o
	gcc $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o : %.c $(HDRS) Makefile

clean:
	rm -f *.o $(BINS) time.tmp outp.tmp

test:
	perl test.t

.PHONY: clean test
