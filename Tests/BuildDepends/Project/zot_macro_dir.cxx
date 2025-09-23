#define ZOT_DIR(x) <zot_##x##_dir.hxx>
#include ZOT_DIR(macro)

char const* zot_macro_dir_f()
{
  return zot_macro_dir;
}
