/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/*
 * Fuzzer for CMake's Fortran dependency scanner
 *
 * CMake runs a flex lexer and a bison parser over Fortran sources to discover
 * module dependencies: MODULE, SUBMODULE, USE and INCLUDE statements, plus the
 * preprocessor directives that guard them. This fuzzer drives that scanner.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <set>
#include <string>
#include <vector>

#include <unistd.h>

#include "cmFortranParser.h"
#include "cmSystemTools.h"

static constexpr size_t kMaxInputSize = 256 * 1024;
static std::string g_testDir;

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
  (void)argc;
  (void)argv;

  char tmpl[] = "/tmp/cmake_fuzz_fortran_XXXXXX";
  char* dir = mkdtemp(tmpl);
  if (dir) {
    g_testDir = dir;
  } else {
    g_testDir = "/tmp/cmake_fuzz_fortran";
    cmSystemTools::MakeDirectory(g_testDir);
  }

  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size)
{
  if (size == 0 || size > kMaxInputSize) {
    return 0;
  }

  std::string const source = g_testDir + "/test.f90";
  {
    FILE* fp = fopen(source.c_str(), "wb");
    if (!fp) {
      return 0;
    }
    fwrite(data, 1, size, fp);
    fclose(fp);
  }

  cmFortranCompiler compiler;
  compiler.Id = "GNU";
  compiler.SModSep = "@";
  compiler.SModExt = ".smod";

  cmFortranSourceInfo info;
  info.Source = source;

  std::vector<std::string> const includes;
  std::set<std::string> const defines;

  cmFortranParser parser(compiler, includes, defines, info);
  if (cmFortranParser_FilePush(&parser, source.c_str())) {
    cmFortran_yyparse(parser.Scanner);
  }

  unlink(source.c_str());

  return 0;
}
