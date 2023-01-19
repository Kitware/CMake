#ifdef _MSC_VER
// Only MSVC supports this pattern.
module M:internal_part;
#else
module M;
#endif

int i()
{
  return 0;
}
