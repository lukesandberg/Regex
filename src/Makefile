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
	$(ECHO) cleaning lib
	$(RM) -f $(OBJS)
	$(RM) $(LIB)

depend: Makefile $(SRCS)
	makedepend -Y $(INCLUDES) $(SRCS) Makefile
	
# DO NOT DELETE

capture_group.o: ./re.h ./capture_group.h
re_ast.o: re_ast.h ./util/linked_list.h ./util/util.h
re_compiler.o: ./re_compiler.h ./vm.h ./re.h ./re_parser.h re_ast.h
re_compiler.o: ./util/linked_list.h ./util/util.h
re_error.o: ./re.h
regex.o: ./re_compiler.h ./vm.h ./re.h re_ast.h ./util/linked_list.h
regex.o: ./util/util.h ./util/sparse_map.h ./thread_state.h ./capture_group.h
re_lexer.o: ./re_lexer.h
re_parser.o: ./re_parser.h re_ast.h ./util/linked_list.h ./re.h ./re_lexer.h
re_parser.o: ./util/fat_stack.h ./util/util.h
vm.o: ./vm.h
util/fat_stack.o: util/fat_stack.h util/util.h
util/linked_list.o: util/linked_list.h
util/sparse_map.o: util/sparse_map.h
util/stack.o: ./util/stack.h ./util/util.h
util/util.o: ./util/util.h
