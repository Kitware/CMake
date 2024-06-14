#include "pcShared.h"
extern const char* pcStatic(void);
int main(void)
{
  pcStatic();
  pcShared();
  return 0;
}
