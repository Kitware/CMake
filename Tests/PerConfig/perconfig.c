#include "pcShared.h"
extern char const* pcStatic(void);
int main(void)
{
  pcStatic();
  pcShared();
  return 0;
}
