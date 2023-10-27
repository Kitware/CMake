#include <uv.h>

int main(void)
{
  uv_loop_close(uv_default_loop());
  return 0;
}
