#ifndef __MIN_UNIT_H__
#define __MIN_UNIT_H__

//super simple c unit testing framework based on this
//from http://www.jera.com/techinfo/jtns/jtn002.html


#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); mu_process_result(#test, message); } while (0)
extern int tests_run;
extern int verbose;
extern int exit_early;
void mu_process_result(char* tn, char*result);
void mu_print_summary();
#endif
