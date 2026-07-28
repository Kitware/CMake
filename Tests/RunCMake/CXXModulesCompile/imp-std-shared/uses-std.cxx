export module uses_std;

import std;

#ifdef _WIN32
export __declspec(dllexport) int dummy()
{
  return 0;
}
#endif

export inline int use_chrono()
{
  static_cast<void>(std::chrono::current_zone());
  return 42;
}
