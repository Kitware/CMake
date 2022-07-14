#include <cstdio>

#include "dxc/dxcapi.h"
#include "printf.h"

int main()
{
  IDxcCompiler3* compiler;
  DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

  assert(compiler);

  IDxcVersionInfo* version;
  compiler->QueryInterface(&version);

  uint32_t major, minor;
  version->GetVersion(&major, &minor);
  printf("DirectX Shader Compiler: %u.%u\n", major, minor);
  version->Release();
  compiler->Release();

  return 0;
}
