include ../build/Makefile.inc

SRCS=$(wildcard *.c util/*.c)
OBJS=$(SRCS:.c=.o)
INCLUDES=-I.
CFLAGS=$(PRJCFLAGS) $(INCLUDES)

LIB=../build/$(LIBNAME)

all: $(LIB)

$(LIB) : $(OBJS)
	$(AR) $(ARFLAGS) r $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	$(RM) -f $(OBJS)
	$(RM) $(LIB)

depend: Makefile $(SRCS)
	makedepend -Y $(INCLUDES) $(SRCS) Makefile
	
# DO NOT DELETE

capture_group.o: ./re.h ./capture_group.h ./util/util.h
re_ast.o: re_ast.h ./util/linked_list.h ./util/util.h
re_compiler.o: ./re_compiler.h ./vm.h ./re.h re_ast.h ./util/linked_list.h
re_compiler.o: ./re_parser.h ./util/util.h regex.h
re_error.o: ./re.h
regex.o: ./re_compiler.h ./vm.h ./re.h re_ast.h ./util/linked_list.h
regex.o: ./util/util.h ./util/sparse_map.h ./thread_state.h ./re_parser.h
regex.o: ./capture_group.h regex.h
re_lexer.o: ./re_lexer.h
re_parser.o: ./re_parser.h re_ast.h ./util/linked_list.h ./re.h ./re_lexer.h
re_parser.o: ./util/fat_stack.h ./util/util.h
vm.o: ./vm.h
util/fat_stack.o: util/util.h util/fat_stack.h
util/linked_list.o: util/linked_list.h ./util/util.h
util/sparse_map.o: util/sparse_map.h ./util/util.h
util/stack.o: ./util/stack.h ./util/util.h
util/util.o: ./util/util.h
