/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/*
 * Fuzzer for CMakePresets.json / CMakeUserPresets.json handling
 *
 * cmJSONParserFuzzer already covers the jsoncpp syntax layer. This fuzzer
 * targets the layer above it, cmCMakePresetsGraph, which turns a parsed
 * document into a resolved preset graph.
 *
 * That code runs before any build logic does: `cmake --list-presets` reads
 * these files without configuring anything, and IDEs parse them to populate
 * their UI as soon as a folder is opened. The input is therefore untrusted
 * data rather than the trusted, executable build code that CMakeLists.txt is.
 *
 * Coverage targets:
 * - schema validation of configure/build/test/package/workflow presets
 * - "include" chains between preset files, and their cycle detection
 * - "inherits" resolution, and its cycle detection
 * - macro expansion ($env{}, $penv{}, ${sourceDir}, ...)
 * - version gating and the project/user file interaction
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <string>

#include <unistd.h>

#include "cmCMakePresetsGraph.h"
#include "cmMessageMetadata.h"
#include "cmSystemTools.h"

static constexpr size_t kMaxInputSize = 64 * 1024;

static std::string g_testDir;
static std::string g_projectFile;
static std::string g_userFile;
static std::string g_explicitFile;
static std::string g_includedFile;

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
  (void)argc;
  (void)argv;

  // Suppress output during fuzzing (set once at init)
  cmSystemTools::SetMessageCallback(
    [](std::string const&, cmMessageMetadata const&) {});
  cmSystemTools::SetStdoutCallback([](std::string const&) {});
  cmSystemTools::SetStderrCallback([](std::string const&) {});

  char tmpl[] = "/tmp/cmake_fuzz_presets_XXXXXX";
  char* dir = mkdtemp(tmpl);
  if (dir) {
    g_testDir = dir;
  } else {
    g_testDir = "/tmp/cmake_fuzz_presets";
    cmSystemTools::MakeDirectory(g_testDir);
  }

  g_projectFile = g_testDir + "/CMakePresets.json";
  g_userFile = g_testDir + "/CMakeUserPresets.json";
  g_explicitFile = g_testDir + "/explicit.json";
  g_includedFile = g_testDir + "/included.json";

  return 0;
}

namespace {

// Which root file is on disk decides which reader runs, so the layout *is* the
// mode -- see the comment in RunOneLayout().
enum class RootLayout
{
  Project,
  User,
  Explicit,
};

bool WriteWholeFile(std::string const& path, uint8_t const* data, size_t size)
{
  FILE* fp = fopen(path.c_str(), "wb");
  if (!fp) {
    return false;
  }
  bool const ok = fwrite(data, 1, size, fp) == size;
  fclose(fp);
  return ok;
}

// Run the whole pipeline once, for one root layout, from a clean directory.
void RunOneLayout(RootLayout layout, uint8_t const* data, size_t size)
{
  unlink(g_projectFile.c_str());
  unlink(g_userFile.c_str());
  unlink(g_explicitFile.c_str());

  std::string presetsFileArg;
  switch (layout) {
    case RootLayout::Project:
      if (!WriteWholeFile(g_projectFile, data, size)) {
        return;
      }
      break;
    case RootLayout::User:
      if (!WriteWholeFile(g_userFile, data, size)) {
        return;
      }
      break;
    case RootLayout::Explicit:
      if (!WriteWholeFile(g_explicitFile, data, size)) {
        return;
      }
      presetsFileArg = g_explicitFile;
      break;
  }

  // A fresh graph per layout: ClearPresets() empties the preset maps but
  // leaves "errors" and "parseState" behind, so a reused graph would carry
  // diagnostic state from one layout into the next.
  cmCMakePresetsGraph graph;
  if (!graph.ReadProjectPresets(g_testDir, presetsFileArg)) {
    return;
  }

  // Walking the graph reaches the inheritance and macro-expansion results, not
  // just the parse that produced them.
  for (auto const& it : graph.ConfigurePresets) {
    (void)graph.GetGeneratorForPreset(it.first);
    (void)it.second.Expanded.has_value();
  }
  for (auto const& it : graph.BuildPresets) {
    (void)graph.GetGeneratorForPreset(it.first);
    (void)it.second.Expanded.has_value();
  }
  for (auto const& it : graph.TestPresets) {
    (void)graph.GetGeneratorForPreset(it.first);
    (void)it.second.Expanded.has_value();
  }
  for (auto const& it : graph.PackagePresets) {
    (void)it.second.Expanded.has_value();
  }
  for (auto const& it : graph.WorkflowPresets) {
    (void)it.second.Expanded.has_value();
  }
}

}

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size)
{
  if (size == 0 || size > kMaxInputSize) {
    return 0;
  }

  if (!WriteWholeFile(g_includedFile, data, size)) {
    return 0;
  }

  for (RootLayout layout :
       { RootLayout::Project, RootLayout::User, RootLayout::Explicit }) {
    RunOneLayout(layout, data, size);
  }

  return 0;
}
