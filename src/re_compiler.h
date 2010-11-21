#ifndef __RE_COMPILER_H__
#define __RE_COMPILER_H__

#include <vm.h>
#include <re.h>
#include <re_ast.h>
regex* compile_regex(ast_node* tree);

#endif

