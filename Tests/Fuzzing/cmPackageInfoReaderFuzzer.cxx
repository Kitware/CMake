/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/*
 * Fuzzer for CMake's CPS (Common Package Specification) reader.
 *
 * cmPackageInfoReader.cxx is 694 lines at 0.00% coverage. It parses the .cps
 * JSON files that third-party packages install and that find_package() picks
 * up from whatever is on the search path, so the input is supplied by the
 * packages found on the machine rather than by the project being built.
 *
 * cmJSONParserFuzzer already covers jsoncpp itself. What is untested is
 * everything the reader does with the decoded value: schema-version gating,
 * prefix resolution against the file's own location, the "simple"/pep440
 * version grammar, requirement specs, and -- the bulk of the file -- turning
 * each entry under "components" into an imported target, with per-
 * configuration properties, link/compile features, definitions, file sets and
 * C++ module metadata.
 *
 * Reaching the target-import half needs a real cmMakefile, so each input gets
 * a fresh cmake/cmGlobalGenerator/cmMakefile fixture. That also keeps targets
 * created by one input from colliding with the next.
 *
 * The reader resolves its prefix by matching "cps_path" against the directory
 * the file was read from, so the input has to be a real file in a real
 * directory: the fixture writes it to <tmp>/cps/fuzz.cps, mirroring the layout
 * the upstream CPS tests use ("cps_path": "@prefix@/cps").
 *
 * Both the primary and the appendix read run on every input. An appendix is a
 * supplemental file read with an already-parsed package as its parent, which
 * skips the schema-version check and inherits the parent's prefix, components
 * and default configurations -- a different path through Read() that no
 * primary read can reach.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>

#include <cm/memory>
#include <cm/optional>

#include "cmExecutionStatus.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmMessenger.h"
#include "cmPackageInfoReader.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmSystemTools.h"
#include "cmTargetTypes.h"
#include "cmake.h"

static constexpr size_t kMaxInputSize = 256 * 1024;

static std::string g_prefixDir;
static std::string g_cpsFile;

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
  (void)argc;
  (void)argv;

  cmSystemTools::SetMessageCallback(
    [](std::string const&, cmMessageMetadata const&) {});
  cmSystemTools::SetStdoutCallback([](std::string const&) {});
  cmSystemTools::SetStderrCallback([](std::string const&) {});

  char tmpl[] = "/tmp/cmake_fuzz_cps_XXXXXX";
  char* dir = mkdtemp(tmpl);
  if (dir) {
    g_prefixDir = dir;
  } else {
    g_prefixDir = "/tmp/cmake_fuzz_cps";
  }

  cmSystemTools::MakeDirectory(g_prefixDir + "/cps");
  g_cpsFile = g_prefixDir + "/cps/fuzz.cps";

  return 0;
}

namespace {

struct Fixture
{
  cmake CMake{ cmState::Role::Project };
  std::unique_ptr<cmGlobalGenerator> GG;
  std::unique_ptr<cmMakefile> MF;

  Fixture()
  {
    this->CMake.SetHomeDirectory(g_prefixDir);
    this->CMake.SetHomeOutputDirectory(g_prefixDir);
    this->GG = cm::make_unique<cmGlobalGenerator>(&this->CMake);
    cmStateSnapshot snapshot = this->CMake.GetCurrentSnapshot();
    snapshot.GetDirectory().SetCurrentBinary(g_prefixDir);
    snapshot.GetDirectory().SetCurrentSource(g_prefixDir);
    this->MF = cm::make_unique<cmMakefile>(this->GG.get(), snapshot);
  }
};

void Consume(cmPackageInfoReader const& reader)
{
  (void)reader.GetName();

  cm::optional<std::string> const version = reader.GetVersion();
  cm::optional<std::string> const compatVersion = reader.GetCompatVersion();
  (void)reader.ParseVersion(version);
  (void)reader.ParseVersion(compatVersion);

  for (cmPackageRequirement const& req : reader.GetRequirements()) {
    (void)req.Name;
    (void)req.Version;
    (void)req.Components;
    (void)req.Hints;
  }
  (void)reader.GetComponentNames();
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size)
{
  if (size == 0 || size > kMaxInputSize) {
    return 0;
  }

  {
    FILE* fp = fopen(g_cpsFile.c_str(), "wb");
    if (!fp) {
      return 0;
    }
    bool const written = fwrite(data, 1, size, fp) == size;
    fclose(fp);
    if (!written) {
      return 0;
    }
  }

  {
    Fixture fx;
    std::unique_ptr<cmPackageInfoReader> reader =
      cmPackageInfoReader::Read(fx.MF.get(), g_cpsFile);
    if (reader) {
      Consume(*reader);

      cmExecutionStatus status(*fx.MF);
      if (reader->ImportTargets(fx.MF.get(), status,
                                cm::ImportedTargetScope::Local)) {
        reader->ImportTargetConfigurations(fx.MF.get(), status);
      }
    }
  }

  {
    Fixture fx;
    std::unique_ptr<cmPackageInfoReader> parent =
      cmPackageInfoReader::Read(fx.MF.get(), g_cpsFile);
    if (parent) {
      std::unique_ptr<cmPackageInfoReader> appendix =
        cmPackageInfoReader::Read(fx.MF.get(), g_cpsFile, parent.get());
      if (appendix) {
        Consume(*appendix);

        cmExecutionStatus status(*fx.MF);
        if (appendix->ImportTargets(fx.MF.get(), status,
                                    cm::ImportedTargetScope::Global)) {
          appendix->ImportTargetConfigurations(fx.MF.get(), status);
        }
      }
    }
  }

  return 0;
}
