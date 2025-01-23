#include <cassert>
#include <iostream>

#include <glslang/Public/ShaderLang.h>

int main()
{
  glslang::Version const glslang_version = glslang::GetVersion();
  char const* glslang_essl_version = glslang::GetEsslVersionString();
  char const* glslang_glsl_version = glslang::GetGlslVersionString();
  int const glslang_khronos_tool_id = glslang::GetKhronosToolId();

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
