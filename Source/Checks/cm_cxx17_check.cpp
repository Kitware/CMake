#include <cstdio>
#include <memory>
#include <unordered_map>

#ifdef _MSC_VER
#  include <comdef.h>
#endif

int main()
{
  std::unique_ptr<int> u(new int(0));

#ifdef _MSC_VER
  // clang-cl has problems instantiating this constructor in C++17 mode
  //  error: indirection requires pointer operand ('const _GUID' invalid)
  //      return *_IID;
  IUnknownPtr ptr{};
  IDispatchPtr disp(ptr);
#endif

  return *u;
}
