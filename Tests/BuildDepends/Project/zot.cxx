#include <stdio.h>
#include <zot.hxx>
#include <zot_custom.hxx>

const char* zot_macro_dir_f();
const char* zot_macro_tgt_f();
const char* zot_pch_f();

int main()
{
  printf("[%s] [%s] [%s] [%s] [%s]\n", zot, zot_custom, zot_macro_dir_f(),
         zot_macro_tgt_f(), zot_pch_f());
  fflush(stdout);
  return 0;
}
