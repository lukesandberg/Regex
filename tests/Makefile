include ../build/Makefile.inc

SRCS=$(wildcard *.c )
OBJS=$(SRCS:.c=.o)
INCLUDES=-I. -I../src
CFLAGS= -g $(INCLUDES)


EXE=../build/$(TESTNAME)
LIB=../build/$(LIBNAME)

all: $(EXE)

$(EXE) : $(OBJS) $(LIB)
	$(CC) $(CFLAGS) $(OBJS) $(LIB) -o $(EXE)
clean:
	$(RM) -f $(OBJS)
	$(RM) -f $(EXE)

depend: Makefile $(SRCS)
	makedepend -Y $(INCLUDES) $(SRCS) Makefile


# DO NOT DELETE

capture_tests.o: capture_tests.h ../src/re.h minunit.h ../src/vm.h
capture_tests.o: ../src/regex.h
compiler_tests.o: compiler_tests.h minunit.h ../src/re_compiler.h ../src/vm.h
compiler_tests.o: ../src/re.h ../src/re_ast.h ../src/util/linked_list.h
compiler_tests.o: ../src/util/util.h ../src/re_parser.h ../src/regex.h
fuzz_tests.o: fuzz_tests.h matcher_tests.h
lexer_tests.o: lexer_tests.h minunit.h ../src/re_lexer.h
main.o: minunit.h lexer_tests.h parser_tests.h matcher_tests.h
main.o: capture_tests.h ../src/re.h compiler_tests.h perf_tests.h
main.o: fuzz_tests.h memory_tests.h ../src/util/util.h
matcher_tests.o: matcher_tests.h minunit.h ../src/re.h ../src/vm.h
matcher_tests.o: ../src/regex.h
memory_tests.o: memory_tests.h ../src/util/util.h minunit.h string_util.h
memory_tests.o: ../src/re.h
minunit.o: minunit.h
parser_tests.o: parser_tests.h minunit.h ../src/re_parser.h ../src/re_ast.h
parser_tests.o: ../src/util/linked_list.h ../src/re.h
perf_tests.o: matcher_tests.h minunit.h string_util.h ../src/re.h
