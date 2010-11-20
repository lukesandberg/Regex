#include <re.h>
#include <capture_group.h>
#include <util/util.h>
char* cg_get_capture(capture_group* cg, unsigned int n, char**end)
{
  *end = cg->regs[2*n+1];
  return cg->regs[2*n];
}
unsigned int cg_get_num_captures(capture_group* cg)
{
  return cg->sz/2;
}
void cg_destroy(capture_group* cap)
{
	rfree(cap);
}