/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/*
 * Fuzzer for CMake's pkg-config resolver.
 *
 * cmPkgConfigParserFuzzer already covers the .pc tokeniser, but it stops at
 * the raw key/value pairs. Everything downstream of that is untested because
 * the resolver is what drives its variable-interpolation entry points.
 *
 * The resolver is where the interesting work happens: recursive ${var}
 * expansion, -I/-L/-l fragment splitting, sysroot re-rooting, and parsing
 * "Requires:" dependency specs with version operators. .pc files come from
 * whatever is on PKG_CONFIG_PATH, so a hostile or corrupt package drop is a
 * realistic source of malformed input.
 *
 * All three strictness levels run on every input. Strictness is not chosen by
 * the input -- Strict and Permissive reject files that BestEffort accepts, so
 * running all three is what reaches the accept/reject paths in
 * HasRequiredFields() and HasStrictConflicts().
 */

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <cm/optional>

#include "cmPkgConfigParser.h"
#include "cmPkgConfigResolver.h"

static constexpr size_t kMaxInputSize = 64 * 1024;

static cmPkgConfigParser ParseInput(uint8_t const* data, size_t size)
{
  // cmPkgConfigParser::Parse takes a non-const buffer (may modify in place)
  std::vector<char> buffer(data, data + size);

  cmPkgConfigParser parser;
  parser.Parse(buffer.data(), buffer.size());
  parser.Finish();
  parser.ParseComplete();

  return parser;
}

static void Consume(cmPkgConfigResolver const& resolver)
{
  (void)resolver.Name();
  (void)resolver.Description();
  (void)resolver.Version();

  // Dependency specs: version operators and the spec grammar.
  for (auto const& spec : resolver.Requires(false)) {
    (void)cmPkgConfigResolver::VersionReqString(spec);
    (void)cmPkgConfigResolver::CheckVersion(spec, "1.2.3");
  }
  (void)resolver.Requires(true);
  (void)resolver.Conflicts();
  (void)resolver.Provides();

  // Fragment splitting and sysroot re-rooting, public and private.
  (void)resolver.Cflags(false);
  (void)resolver.Cflags(true);
  (void)resolver.Libs(false);
  (void)resolver.Libs(true);
}

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size)
{
  if (size == 0 || size > kMaxInputSize) {
    return 0;
  }

  cmPkgConfigEnv env;
  env.SysrootDir = std::string("/fuzz/sysroot");
  env.TopBuildDir = std::string("/fuzz/build");
  env.LibDirs = std::vector<std::string>{ "/usr/lib", "/fuzz/lib" };
  env.SysCflags = std::vector<std::string>{ "/usr/include" };
  env.SysLibs = std::vector<std::string>{ "/usr/lib" };

  std::string const pcFileDir = "/fuzz/pkgconfig";

  cmPkgConfigParser const parsed = ParseInput(data, size);

  if (auto strict =
        cmPkgConfigResolver::ResolveStrict(parsed, env, pcFileDir)) {
    Consume(*strict);
  }

  if (auto permissive =
        cmPkgConfigResolver::ResolvePermissive(parsed, env, pcFileDir)) {
    Consume(*permissive);
  }

  Consume(cmPkgConfigResolver::ResolveBestEffort(parsed, env, pcFileDir));

  return 0;
}
