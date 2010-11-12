include ../build/Makefile.inc

SRCS=$(wildcard *.c )
OBJS=$(SRCS:.c=.o)
INCLUDES=-I. -I../src
CFLAGS=$(PRJCFLAGS) $(INCLUDES)


EXE=../build/$(TESTNAME)
LIB=../build/$(LIBNAME)

all: $(EXE)

$(EXE) : $(OBJS) $(LIB)
	$(CC) $(CFLAGS) $? -o $@
clean:
	$(ECHO) cleaning testb
	$(RM) -f $(OBJS)
	$(RM) -f $(EXE)

depend: Makefile $(SRCS)
	makedepend $(INCLUDES) $(SRCS) Makefile


# DO NOT DELETE

capture_tests.o: capture_tests.h ../src/re.h minunit.h /usr/include/stdlib.h
capture_tests.o: /usr/include/features.h /usr/include/bits/predefs.h
capture_tests.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
capture_tests.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
capture_tests.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
capture_tests.o: /usr/include/endian.h /usr/include/bits/endian.h
capture_tests.o: /usr/include/bits/byteswap.h /usr/include/sys/types.h
capture_tests.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
capture_tests.o: /usr/include/time.h /usr/include/sys/select.h
capture_tests.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
capture_tests.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
capture_tests.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
capture_tests.o: /usr/include/stdio.h /usr/include/libio.h
capture_tests.o: /usr/include/_G_config.h /usr/include/wchar.h
capture_tests.o: /usr/include/bits/stdio_lim.h
capture_tests.o: /usr/include/bits/sys_errlist.h
compiler_tests.o: compiler_tests.h minunit.h ../src/re_compiler.h ../src/vm.h
compiler_tests.o: /usr/include/stdlib.h /usr/include/features.h
compiler_tests.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
compiler_tests.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
compiler_tests.o: /usr/include/gnu/stubs-32.h /usr/include/bits/waitflags.h
compiler_tests.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
compiler_tests.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
compiler_tests.o: /usr/include/sys/types.h /usr/include/bits/types.h
compiler_tests.o: /usr/include/bits/typesizes.h /usr/include/time.h
compiler_tests.o: /usr/include/sys/select.h /usr/include/bits/select.h
compiler_tests.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
compiler_tests.o: /usr/include/sys/sysmacros.h
compiler_tests.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
compiler_tests.o: ../src/re.h /usr/include/stdio.h /usr/include/libio.h
compiler_tests.o: /usr/include/_G_config.h /usr/include/wchar.h
compiler_tests.o: /usr/include/bits/stdio_lim.h
compiler_tests.o: /usr/include/bits/sys_errlist.h
fuzz_tests.o: fuzz_tests.h /usr/include/stdio.h /usr/include/features.h
fuzz_tests.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
fuzz_tests.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
fuzz_tests.o: /usr/include/gnu/stubs-32.h /usr/include/bits/types.h
fuzz_tests.o: /usr/include/bits/typesizes.h /usr/include/libio.h
fuzz_tests.o: /usr/include/_G_config.h /usr/include/wchar.h
fuzz_tests.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
fuzz_tests.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
fuzz_tests.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
fuzz_tests.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
fuzz_tests.o: /usr/include/sys/types.h /usr/include/time.h
fuzz_tests.o: /usr/include/sys/select.h /usr/include/bits/select.h
fuzz_tests.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
fuzz_tests.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
fuzz_tests.o: /usr/include/alloca.h /usr/include/signal.h
fuzz_tests.o: /usr/include/bits/signum.h /usr/include/bits/siginfo.h
fuzz_tests.o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
fuzz_tests.o: /usr/include/bits/sigstack.h /usr/include/sys/ucontext.h
fuzz_tests.o: /usr/include/bits/sigthread.h /usr/include/sys/wait.h
fuzz_tests.o: /usr/include/sys/resource.h /usr/include/bits/resource.h
fuzz_tests.o: /usr/include/string.h /usr/include/xlocale.h
fuzz_tests.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
fuzz_tests.o: /usr/include/bits/environments.h /usr/include/bits/confname.h
fuzz_tests.o: /usr/include/getopt.h matcher_tests.h
lexer_tests.o: lexer_tests.h minunit.h ../src/re_lexer.h /usr/include/stdio.h
lexer_tests.o: /usr/include/features.h /usr/include/bits/predefs.h
lexer_tests.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
lexer_tests.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
lexer_tests.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
lexer_tests.o: /usr/include/libio.h /usr/include/_G_config.h
lexer_tests.o: /usr/include/wchar.h /usr/include/bits/stdio_lim.h
lexer_tests.o: /usr/include/bits/sys_errlist.h
main.o: minunit.h lexer_tests.h parser_tests.h matcher_tests.h
main.o: capture_tests.h ../src/re.h compiler_tests.h perf_tests.h
main.o: fuzz_tests.h /usr/include/stdio.h /usr/include/features.h
main.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
main.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
main.o: /usr/include/gnu/stubs-32.h /usr/include/bits/types.h
main.o: /usr/include/bits/typesizes.h /usr/include/libio.h
main.o: /usr/include/_G_config.h /usr/include/wchar.h
main.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
main.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
main.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
main.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
main.o: /usr/include/sys/types.h /usr/include/time.h
main.o: /usr/include/sys/select.h /usr/include/bits/select.h
main.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
main.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
main.o: /usr/include/alloca.h /usr/include/string.h /usr/include/xlocale.h
matcher_tests.o: matcher_tests.h minunit.h ../src/re.h /usr/include/stdlib.h
matcher_tests.o: /usr/include/features.h /usr/include/bits/predefs.h
matcher_tests.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
matcher_tests.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
matcher_tests.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
matcher_tests.o: /usr/include/endian.h /usr/include/bits/endian.h
matcher_tests.o: /usr/include/bits/byteswap.h /usr/include/sys/types.h
matcher_tests.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
matcher_tests.o: /usr/include/time.h /usr/include/sys/select.h
matcher_tests.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
matcher_tests.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
matcher_tests.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
matcher_tests.o: /usr/include/stdio.h /usr/include/libio.h
matcher_tests.o: /usr/include/_G_config.h /usr/include/wchar.h
matcher_tests.o: /usr/include/bits/stdio_lim.h
matcher_tests.o: /usr/include/bits/sys_errlist.h
minunit.o: minunit.h /usr/include/stdlib.h /usr/include/features.h
minunit.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
minunit.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
minunit.o: /usr/include/gnu/stubs-32.h /usr/include/bits/waitflags.h
minunit.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
minunit.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
minunit.o: /usr/include/sys/types.h /usr/include/bits/types.h
minunit.o: /usr/include/bits/typesizes.h /usr/include/time.h
minunit.o: /usr/include/sys/select.h /usr/include/bits/select.h
minunit.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
minunit.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
minunit.o: /usr/include/alloca.h /usr/include/stdio.h /usr/include/libio.h
minunit.o: /usr/include/_G_config.h /usr/include/wchar.h
minunit.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
parser_tests.o: parser_tests.h minunit.h ../src/re_parser.h ../src/re_ast.h
parser_tests.o: ../src/util/linked_list.h ../src/re.h /usr/include/stdio.h
parser_tests.o: /usr/include/features.h /usr/include/bits/predefs.h
parser_tests.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
parser_tests.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
parser_tests.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
parser_tests.o: /usr/include/libio.h /usr/include/_G_config.h
parser_tests.o: /usr/include/wchar.h /usr/include/bits/stdio_lim.h
parser_tests.o: /usr/include/bits/sys_errlist.h
perf_tests.o: matcher_tests.h minunit.h ../src/re.h /usr/include/stdlib.h
perf_tests.o: /usr/include/features.h /usr/include/bits/predefs.h
perf_tests.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
perf_tests.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
perf_tests.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
perf_tests.o: /usr/include/endian.h /usr/include/bits/endian.h
perf_tests.o: /usr/include/bits/byteswap.h /usr/include/sys/types.h
perf_tests.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
perf_tests.o: /usr/include/time.h /usr/include/sys/select.h
perf_tests.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
perf_tests.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
perf_tests.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
perf_tests.o: /usr/include/string.h /usr/include/xlocale.h
perf_tests.o: /usr/include/stdio.h /usr/include/libio.h
perf_tests.o: /usr/include/_G_config.h /usr/include/wchar.h
perf_tests.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
perf_tests.o: /usr/include/sys/time.h
