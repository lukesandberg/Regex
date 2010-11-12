include ../build/Makefile.inc

SRCS=$(wildcard *.c util/*.c)
OBJS=$(SRCS:.c=.o)
INCLUDES=-I.
CFLAGS=$(PRJCFLAGS) $(INCLUDES)

LIB=../build/$(LIBNAME)

all: $(LIB)

$(LIB) : $(OBJS)
	$(AR) $(ARFLAGS) r $(LIB) $?
	$(RANLIB) $(LIB)

clean:
	$(ECHO) cleaning lib
	$(RM) -f $(OBJS)
	$(RM) $(LIB)

depend: Makefile $(SRCS)
	makedepend $(INCLUDES) $(SRCS) Makefile
	
# DO NOT DELETE

capture_group.o: ./capture_group.h
re_ast.o: re_ast.h ./util/linked_list.h /usr/include/stdlib.h
re_ast.o: /usr/include/features.h /usr/include/bits/predefs.h
re_ast.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
re_ast.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
re_ast.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
re_ast.o: /usr/include/endian.h /usr/include/bits/endian.h
re_ast.o: /usr/include/bits/byteswap.h /usr/include/sys/types.h
re_ast.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
re_ast.o: /usr/include/time.h /usr/include/sys/select.h
re_ast.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
re_ast.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
re_ast.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
re_ast.o: ./util/util.h /usr/include/assert.h
re_compiler.o: ./re_compiler.h ./vm.h /usr/include/stdlib.h
re_compiler.o: /usr/include/features.h /usr/include/bits/predefs.h
re_compiler.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
re_compiler.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
re_compiler.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
re_compiler.o: /usr/include/endian.h /usr/include/bits/endian.h
re_compiler.o: /usr/include/bits/byteswap.h /usr/include/sys/types.h
re_compiler.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
re_compiler.o: /usr/include/time.h /usr/include/sys/select.h
re_compiler.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
re_compiler.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
re_compiler.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h ./re.h
re_compiler.o: ./re_parser.h re_ast.h ./util/linked_list.h ./util/util.h
re_compiler.o: /usr/include/string.h /usr/include/xlocale.h
re_error.o: ./re.h /usr/include/stdio.h /usr/include/features.h
re_error.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
re_error.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
re_error.o: /usr/include/gnu/stubs-32.h /usr/include/bits/types.h
re_error.o: /usr/include/bits/typesizes.h /usr/include/libio.h
re_error.o: /usr/include/_G_config.h /usr/include/wchar.h
re_error.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
regex.o: ./re_compiler.h ./vm.h /usr/include/stdlib.h /usr/include/features.h
regex.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
regex.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
regex.o: /usr/include/gnu/stubs-32.h /usr/include/bits/waitflags.h
regex.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
regex.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
regex.o: /usr/include/sys/types.h /usr/include/bits/types.h
regex.o: /usr/include/bits/typesizes.h /usr/include/time.h
regex.o: /usr/include/sys/select.h /usr/include/bits/select.h
regex.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
regex.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
regex.o: /usr/include/alloca.h ./re.h re_ast.h ./util/linked_list.h
regex.o: ./util/util.h ./util/sparse_map.h ./thread_state.h
regex.o: /usr/include/string.h /usr/include/xlocale.h /usr/include/stdint.h
regex.o: /usr/include/bits/wchar.h /usr/include/ctype.h /usr/include/stdio.h
regex.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
regex.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
re_lexer.o: ./re_lexer.h /usr/include/stdio.h /usr/include/features.h
re_lexer.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
re_lexer.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
re_lexer.o: /usr/include/gnu/stubs-32.h /usr/include/bits/types.h
re_lexer.o: /usr/include/bits/typesizes.h /usr/include/libio.h
re_lexer.o: /usr/include/_G_config.h /usr/include/wchar.h
re_lexer.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
re_lexer.o: /usr/include/ctype.h /usr/include/endian.h
re_lexer.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
re_lexer.o: /usr/include/xlocale.h
re_parser.o: ./re_parser.h re_ast.h ./util/linked_list.h ./re.h ./re_lexer.h
re_parser.o: /usr/include/stdlib.h /usr/include/features.h
re_parser.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
re_parser.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
re_parser.o: /usr/include/gnu/stubs-32.h /usr/include/bits/waitflags.h
re_parser.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
re_parser.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
re_parser.o: /usr/include/sys/types.h /usr/include/bits/types.h
re_parser.o: /usr/include/bits/typesizes.h /usr/include/time.h
re_parser.o: /usr/include/sys/select.h /usr/include/bits/select.h
re_parser.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
re_parser.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
re_parser.o: /usr/include/alloca.h /usr/include/stdio.h /usr/include/libio.h
re_parser.o: /usr/include/_G_config.h /usr/include/wchar.h
re_parser.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
re_parser.o: ./util/fat_stack.h ./util/util.h
thread_state.o: ./thread_state.h /usr/include/stdlib.h
thread_state.o: /usr/include/features.h /usr/include/bits/predefs.h
thread_state.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
thread_state.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
thread_state.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
thread_state.o: /usr/include/endian.h /usr/include/bits/endian.h
thread_state.o: /usr/include/bits/byteswap.h /usr/include/sys/types.h
thread_state.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
thread_state.o: /usr/include/time.h /usr/include/sys/select.h
thread_state.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
thread_state.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
thread_state.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
thread_state.o: /usr/include/string.h /usr/include/xlocale.h
thread_state.o: /usr/include/stdint.h /usr/include/bits/wchar.h
vm.o: ./vm.h /usr/include/stdlib.h /usr/include/features.h
vm.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
vm.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
vm.o: /usr/include/gnu/stubs-32.h /usr/include/bits/waitflags.h
vm.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
vm.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
vm.o: /usr/include/sys/types.h /usr/include/bits/types.h
vm.o: /usr/include/bits/typesizes.h /usr/include/time.h
vm.o: /usr/include/sys/select.h /usr/include/bits/select.h
vm.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
vm.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
vm.o: /usr/include/alloca.h /usr/include/stdio.h /usr/include/libio.h
vm.o: /usr/include/_G_config.h /usr/include/wchar.h
vm.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
util/fat_stack.o: /usr/include/stdlib.h /usr/include/features.h
util/fat_stack.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
util/fat_stack.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
util/fat_stack.o: /usr/include/gnu/stubs-32.h /usr/include/bits/waitflags.h
util/fat_stack.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
util/fat_stack.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
util/fat_stack.o: /usr/include/sys/types.h /usr/include/bits/types.h
util/fat_stack.o: /usr/include/bits/typesizes.h /usr/include/time.h
util/fat_stack.o: /usr/include/sys/select.h /usr/include/bits/select.h
util/fat_stack.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
util/fat_stack.o: /usr/include/sys/sysmacros.h
util/fat_stack.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
util/fat_stack.o: /usr/include/string.h /usr/include/xlocale.h
util/fat_stack.o: util/fat_stack.h util/util.h
util/linked_list.o: /usr/include/stdlib.h /usr/include/features.h
util/linked_list.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
util/linked_list.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
util/linked_list.o: /usr/include/gnu/stubs-32.h /usr/include/bits/waitflags.h
util/linked_list.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
util/linked_list.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
util/linked_list.o: /usr/include/sys/types.h /usr/include/bits/types.h
util/linked_list.o: /usr/include/bits/typesizes.h /usr/include/time.h
util/linked_list.o: /usr/include/sys/select.h /usr/include/bits/select.h
util/linked_list.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
util/linked_list.o: /usr/include/sys/sysmacros.h
util/linked_list.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
util/linked_list.o: util/linked_list.h
util/sparse_map.o: util/sparse_map.h /usr/include/stdlib.h
util/sparse_map.o: /usr/include/features.h /usr/include/bits/predefs.h
util/sparse_map.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
util/sparse_map.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
util/sparse_map.o: /usr/include/bits/waitflags.h
util/sparse_map.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
util/sparse_map.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
util/sparse_map.o: /usr/include/sys/types.h /usr/include/bits/types.h
util/sparse_map.o: /usr/include/bits/typesizes.h /usr/include/time.h
util/sparse_map.o: /usr/include/sys/select.h /usr/include/bits/select.h
util/sparse_map.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
util/sparse_map.o: /usr/include/sys/sysmacros.h
util/sparse_map.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
util/stack.o: /usr/include/stdlib.h /usr/include/features.h
util/stack.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
util/stack.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
util/stack.o: /usr/include/gnu/stubs-32.h /usr/include/bits/waitflags.h
util/stack.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
util/stack.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
util/stack.o: /usr/include/sys/types.h /usr/include/bits/types.h
util/stack.o: /usr/include/bits/typesizes.h /usr/include/time.h
util/stack.o: /usr/include/sys/select.h /usr/include/bits/select.h
util/stack.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
util/stack.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
util/stack.o: /usr/include/alloca.h ./util/stack.h ./util/util.h
util/util.o: ./util/util.h /usr/include/stdlib.h /usr/include/features.h
util/util.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
util/util.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
util/util.o: /usr/include/gnu/stubs-32.h /usr/include/bits/waitflags.h
util/util.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
util/util.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
util/util.o: /usr/include/sys/types.h /usr/include/bits/types.h
util/util.o: /usr/include/bits/typesizes.h /usr/include/time.h
util/util.o: /usr/include/sys/select.h /usr/include/bits/select.h
util/util.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
util/util.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
util/util.o: /usr/include/alloca.h /usr/include/stdio.h /usr/include/libio.h
util/util.o: /usr/include/_G_config.h /usr/include/wchar.h
util/util.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
util/util.o: /usr/include/string.h /usr/include/xlocale.h
