#ifdef _MSC_VER
// Only MSVC supports this pattern.
module M:part;
#else
module M;
#endif

import M:internal_part;

int p()
{
  return i();
}
