#ifndef _CAPTURE_TESTS_H_
#define _CAPTURE_TESTS_H_
#include <capture_group.h>
void test_captures();
int capture(char* re_str, char* str, capture_group** caps);
#endif
