#include <re.h>
#include <capture_group.h>

char* cg_get_capture(capture_group* cg, unsigned int n, char**end)
{
  *end = cg->regs[n+1];
  return cg->regs[n];
}
unsigned int cg_get_num_captures(capture_group* cg)
{
  return cg->sz/2;
}