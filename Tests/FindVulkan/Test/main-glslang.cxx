#include <cassert>
#include <iostream>

#include <glslang/Public/ShaderLang.h>

int main()
{
  const glslang::Version glslang_version = glslang::GetVersion();
  const char* glslang_essl_version = glslang::GetEsslVersionString();
  const char* glslang_glsl_version = glslang::GetGlslVersionString();
  const int glslang_khronos_tool_id = glslang::GetKhronosToolId();

  std::cout << "glslang Version: " << glslang_version.major << '.'
            << glslang_version.minor << '.' << glslang_version.patch
            << " (glsl version: " << glslang_glsl_version
            << ", essl version:" << glslang_essl_version
            << ", khronos tool:" << glslang_khronos_tool_id << ')'
            << std::endl;

  assert(glslang_essl_version);
  assert(glslang_glsl_version);

  return 0;
}
