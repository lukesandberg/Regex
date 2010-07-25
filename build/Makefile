.PHONY: clean

TARGET = testre

LIBSRCS=$(wildcard ../src/*.c )
SRCS=$(wildcard ../tests/*.c )
INCLUDES=-I../src -I../tests
OBJECTS=$(SRCS:.c=.o)
LIBOBJECTS=$(LIBSRCS:.c=.o)

CC =gcc -std=c99
CFLAGS =-Wall -Werror -pedantic -g  $(INCLUDES)


$(TARGET): lib $(OBJECTS)
	$(CC) $(CFLAGS) $(LIBOBJECTS) $(OBJECTS) -o $@

lib: $(LIBOBJECTS)

clean:
	rm -f $(TARGET) $(LIBOBJECTS) $(OBJECTS)

test: $(TARGET)
	bash -c ./$(TARGET)	

depend:	Makefile $(SRCS)
	makedepend $(INCLUDES) $(SRCS) Makefile


# DO NOT DELETE

../tests/main.o: ../tests/minunit.h ../src/regex.h ../src/parse.h
../tests/minunit.o: ../tests/minunit.h /usr/include/stdlib.h
../tests/minunit.o: /usr/include/features.h /usr/include/bits/predefs.h
../tests/minunit.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
../tests/minunit.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
../tests/minunit.o: /usr/include/sys/types.h /usr/include/bits/types.h
../tests/minunit.o: /usr/include/bits/typesizes.h /usr/include/time.h
../tests/minunit.o: /usr/include/endian.h /usr/include/bits/endian.h
../tests/minunit.o: /usr/include/bits/byteswap.h /usr/include/sys/select.h
../tests/minunit.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
../tests/minunit.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
../tests/minunit.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
../tests/minunit.o: /usr/include/stdio.h /usr/include/libio.h
../tests/minunit.o: /usr/include/_G_config.h /usr/include/wchar.h
../tests/minunit.o: /usr/include/bits/stdio_lim.h
../tests/minunit.o: /usr/include/bits/sys_errlist.h
