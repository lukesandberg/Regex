#ifndef __MIN_UNIT_H__
#define __MIN_UNIT_H__

//super simple c unit testing framework based on this
//from http://www.jera.com/techinfo/jtns/jtn002.html


#define mu_assert(message, test) do \
				{ \
					if (!(test))\
					{\
						mu_fail(message, __FILE__, __LINE__);\
						return 0;\
					}\
				} while (0)

#define mu_run_test(test) do\
			{ \
				mu_start_test(#test);\
				if(test()) mu_succeed();\
			} while (0)

extern int tests_run;
extern int verbose;
void mu_start_test();
void mu_succeed();
void mu_fail(char* msg, char* filename, unsigned int ln);
void mu_print_summary();
#endif
