/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/*
 * Fuzzer for generator expression evaluation.
 *
 * cmGeneratorExpressionFuzzer only tokenizes and preprocesses $<...>, and
 * cmScriptFuzzer runs in script mode where no generator exists, so the
 * evaluator (cmGeneratorExpressionNode.cxx, the DAG checker, and the
 * transitive-property / link-interface code in cmGeneratorTarget) is never
 * reached by any fuzzer.
 *
 * Each input gets a fresh project-mode fixture: a base cmGlobalGenerator, a
 * makefile populated from a fixed listfile (interface, imported, alias,
 * object, static, executable and custom targets with genex-bearing usage
 * requirements), and generator targets created the same way as
 * "cmake --find-package" does.  C and CXX are marked enabled with
 * GNU-like compiler variables so that language- and compiler-dependent
 * expressions have something to evaluate against.  No compiler is run.
 *
 * Input layout:
 *   byte 0: bits 0-1 mode, bits 2-4 head target, bits 5-6 config,
 *           bit 7 EvaluateForBuildsystem
 *   byte 1: bits 0-2 language, bits 3-4 property, bits 5-7 current target
 *   rest:   generator expression (or property value in Injection mode)
 *
 * Modes (see Mode below):
 *   Evaluate     cmGeneratorExpression::Evaluate
 *   Compiled     Parse once, evaluate for two configs, query collected
 * metadata Interpreter  cmGeneratorExpressionInterpreter with a
 * transitive-property DAG Injection    Store the input in a usage-requirement
 * property of a fixture target, then read it back through a consumer so it is
 *                evaluated transitively
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include <cm/memory>

#include "cmGenExContext.h"
#include "cmGeneratorExpression.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageMetadata.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmake.h"

static constexpr size_t kHeaderSize = 2;
// Evaluation is recursive in nesting depth; keep inputs short.
static constexpr size_t kMaxExprSize = 4 * 1024;

static std::string g_srcDir;
static std::string g_binDir;

namespace {

// Stand-in for the variables project() would load after compiler detection.
char const kToolchain[] = R"cmake(
cmake_minimum_required(VERSION 4.0)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_BUILD_TYPE Debug)
set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)
set(CMAKE_STATIC_LIBRARY_PREFIX lib)
set(CMAKE_STATIC_LIBRARY_SUFFIX .a)
set(CMAKE_SHARED_LIBRARY_PREFIX lib)
set(CMAKE_SHARED_LIBRARY_SUFFIX .so)
set(CMAKE_SHARED_MODULE_PREFIX lib)
set(CMAKE_SHARED_MODULE_SUFFIX .so)
set(CMAKE_EXECUTABLE_SUFFIX "")
foreach(lang IN ITEMS C CXX)
  set(CMAKE_${lang}_COMPILER_ID GNU)
  set(CMAKE_${lang}_COMPILER_VERSION 13.2.0)
  set(CMAKE_${lang}_COMPILER_FRONTEND_VARIANT GNU)
  set(CMAKE_${lang}_COMPILER_LINKER_ID GNU)
  set(CMAKE_${lang}_COMPILER_LINKER_FRONTEND_VARIANT GNU)
  set(CMAKE_${lang}_OUTPUT_EXTENSION .o)
  set(CMAKE_${lang}_STANDARD_DEFAULT 17)
endforeach()
set(CMAKE_C_COMPILER /usr/bin/cc)
set(CMAKE_C_SOURCE_FILE_EXTENSIONS c m)
set(CMAKE_C_IGNORE_EXTENSIONS h H o O obj OBJ def DEF rc RC)
set(CMAKE_C_LINKER_PREFERENCE 10)
set(CMAKE_C_COMPILE_FEATURES
  c_std_90 c_std_99 c_std_11 c_std_17 c_std_23
  c_function_prototypes c_restrict c_static_assert c_variadic_macros)
set(CMAKE_CXX_COMPILER /usr/bin/c++)
set(CMAKE_CXX_SOURCE_FILE_EXTENSIONS
  C M c++ cc cpp cxx mm mpp CPP ixx cppm ccm cxxm c++m)
set(CMAKE_CXX_IGNORE_EXTENSIONS inl h hpp HPP H o O obj OBJ def DEF rc RC)
set(CMAKE_CXX_LINKER_PREFERENCE 30)
set(CMAKE_CXX_LINKER_PREFERENCE_PROPAGATES 1)
set(CMAKE_CXX_COMPILE_FEATURES
  cxx_std_98 cxx_std_11 cxx_std_14 cxx_std_17 cxx_std_20 cxx_std_23
  cxx_auto_type cxx_constexpr cxx_lambdas cxx_nullptr cxx_variadic_templates)
)cmake";

char const kProject[] = R"cmake(
cmake_minimum_required(VERSION 4.0)

add_library(fuzz_iface INTERFACE)
target_compile_definitions(fuzz_iface INTERFACE
  IFACE_DEF=1
  $<$<CONFIG:Debug>:IFACE_DEBUG>
  $<$<COMPILE_LANGUAGE:CXX>:IFACE_CXX>)
target_include_directories(fuzz_iface INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_options(fuzz_iface INTERFACE
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-Wall>)
target_link_options(fuzz_iface INTERFACE
  $<$<LINK_LANGUAGE:CXX>:-Wl,--as-needed>)
target_sources(fuzz_iface INTERFACE
  FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/fuzz.h)
set_target_properties(fuzz_iface PROPERTIES
  INTERFACE_FUZZ_BOOL ON
  INTERFACE_FUZZ_STRING fuzz
  INTERFACE_FUZZ_NUMBER 3
  INTERFACE_FUZZ_CUSTOM "custom;$<CONFIG>"
  COMPATIBLE_INTERFACE_BOOL FUZZ_BOOL
  COMPATIBLE_INTERFACE_STRING FUZZ_STRING
  COMPATIBLE_INTERFACE_NUMBER_MAX FUZZ_NUMBER
  TRANSITIVE_COMPILE_PROPERTIES FUZZ_CUSTOM)

add_library(fuzz_dep INTERFACE)
target_link_libraries(fuzz_dep INTERFACE
  fuzz_iface
  $<LINK_ONLY:fuzz_imported_static>
  $<$<CONFIG:Release>:Fuzz::shared>)

add_library(fuzz_imported_static STATIC IMPORTED)
set_target_properties(fuzz_imported_static PROPERTIES
  IMPORTED_CONFIGURATIONS "DEBUG;RELEASE"
  IMPORTED_LOCATION_DEBUG /opt/fuzz/lib/libfuzz_d.a
  IMPORTED_LOCATION_RELEASE /opt/fuzz/lib/libfuzz.a
  MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
  IMPORTED_LINK_INTERFACE_LANGUAGES C
  INTERFACE_COMPILE_DEFINITIONS IMPORTED_STATIC)

add_library(fuzz_imported_shared SHARED IMPORTED GLOBAL)
set_target_properties(fuzz_imported_shared PROPERTIES
  IMPORTED_LOCATION /opt/fuzz/lib/libfuzz_shared.so.1.2.3
  IMPORTED_SONAME libfuzz_shared.so.1
  INTERFACE_INCLUDE_DIRECTORIES /opt/fuzz/include
  INTERFACE_LINK_LIBRARIES fuzz_imported_static)
add_library(Fuzz::shared ALIAS fuzz_imported_shared)

add_executable(fuzz_imported_exe IMPORTED)
set_property(TARGET fuzz_imported_exe PROPERTY
  IMPORTED_LOCATION /opt/fuzz/bin/fuzz_tool)

add_library(fuzz_obj OBJECT fuzz.c fuzz.cpp)
target_compile_definitions(fuzz_obj PRIVATE OBJ_DEF)

add_library(fuzz_static STATIC fuzz.c)
target_link_libraries(fuzz_static
  PUBLIC fuzz_dep
  PRIVATE fuzz_obj Fuzz::shared)
set_target_properties(fuzz_static PROPERTIES
  OUTPUT_NAME fuzzy
  OUTPUT_NAME_DEBUG fuzzy_d
  POSITION_INDEPENDENT_CODE ON
  COMPILE_DEFINITIONS "STATIC_OWN=$<CONFIG>")
add_library(Fuzz::static ALIAS fuzz_static)

add_executable(fuzz_exe fuzz.cpp)
target_link_libraries(fuzz_exe PRIVATE Fuzz::static)
set_property(TARGET fuzz_exe PROPERTY FUZZ_STRING fuzz)

add_custom_target(fuzz_custom
  COMMAND fuzz_imported_exe $<TARGET_FILE:fuzz_exe>)

set_source_files_properties(fuzz.c PROPERTIES
  COMPILE_DEFINITIONS "SRC_DEF=$<CONFIG>"
  FUZZ_SRC_PROP value)
set_property(GLOBAL PROPERTY FUZZ_GLOBAL "global;value")
set_property(DIRECTORY PROPERTY FUZZ_DIR_PROP dir)
)cmake";

char const* const kConfigs[] = { "", "Debug", "Release", "RelWithDebInfo" };

char const* const kLanguages[] = { "",        "C",   "CXX",  "CUDA",
                                   "Fortran", "HIP", "OBJC", "Swift" };

// Index 0 means "no target".
char const* const kTargets[] = {
  nullptr,        "fuzz_exe",    "fuzz_static",
  "fuzz_obj",     "fuzz_iface",  "fuzz_imported_static",
  "Fuzz::shared", "fuzz_custom",
};

char const* const kFixtureTargets[] = {
  "fuzz_exe",          "fuzz_static",          "Fuzz::static",
  "fuzz_obj",          "fuzz_iface",           "fuzz_dep",
  "fuzz_imported_exe", "fuzz_imported_static", "fuzz_imported_shared",
  "Fuzz::shared",      "fuzz_custom",
};

// Properties that generators pass to cmGeneratorExpressionInterpreter.
char const* const kInterpreterProperties[] = {
  "COMPILE_OPTIONS",
  "COMPILE_DEFINITIONS",
  "INCLUDE_DIRECTORIES",
  "COMPILE_FLAGS",
};

struct Injection
{
  char const* Target;
  char const* Property;
  // Evaluated with fuzz_exe as head so the injected value is reached
  // through the link closure.
  char const* Reader;
};

Injection const kInjections[] = {
  { "fuzz_dep", "INTERFACE_COMPILE_DEFINITIONS",
    "$<TARGET_PROPERTY:COMPILE_DEFINITIONS>" },
  { "fuzz_dep", "INTERFACE_INCLUDE_DIRECTORIES",
    "$<TARGET_PROPERTY:INCLUDE_DIRECTORIES>" },
  { "fuzz_dep", "INTERFACE_LINK_LIBRARIES",
    "$<TARGET_PROPERTY:COMPILE_OPTIONS>" },
  { "fuzz_exe", "LINK_LIBRARIES", "$<TARGET_PROPERTY:LINK_OPTIONS>" },
};

// Which evaluation entry point the input exercises (bits 0-1 of byte 0).
enum class Mode
{
  Evaluate,    // cmGeneratorExpression::Evaluate (one-shot)
  Compiled,    // Parse once, evaluate for two configs, query metadata
  Interpreter, // cmGeneratorExpressionInterpreter (generator property path)
  Injection,   // Store in a usage requirement, evaluate transitively
};

bool WriteFile(std::string const& path, char const* content)
{
  FILE* fp = fopen(path.c_str(), "wb");
  if (!fp) {
    return false;
  }
  bool const ok = fputs(content, fp) >= 0;
  fclose(fp);
  return ok;
}

struct Fixture
{
  cmake CMake{ cmState::Role::Project };
  cmGlobalGenerator* GG = nullptr;
  cmMakefile* MF = nullptr;
  cmLocalGenerator* LG = nullptr;

  bool Build(Injection const* inject, std::string const& value)
  {
    this->CMake.SetHomeDirectory(g_srcDir);
    this->CMake.SetHomeOutputDirectory(g_binDir);
    this->CMake.SetGlobalGenerator(
      cm::make_unique<cmGlobalGenerator>(&this->CMake));
    this->GG = this->CMake.GetGlobalGenerator();

    cmStateSnapshot snapshot = this->CMake.GetCurrentSnapshot();
    snapshot.GetDirectory().SetCurrentSource(g_srcDir);
    snapshot.GetDirectory().SetCurrentBinary(g_binDir);
    snapshot.SetDefaultDefinitions();

    auto mf = cm::make_unique<cmMakefile>(this->GG, snapshot);
    this->MF = mf.get();
    this->GG->AddMakefile(std::move(mf));

    if (!this->MF->ReadListFileAsString(kToolchain, "fuzz_toolchain.cmake")) {
      return false;
    }
    this->GG->SetLanguageEnabled("C", this->MF);
    this->GG->SetLanguageEnabled("CXX", this->MF);
    if (!this->MF->ReadListFileAsString(kProject, "CMakeLists.txt") ||
        cmSystemTools::GetFatalErrorOccurred()) {
      return false;
    }

    if (inject) {
      cmTarget* target = this->MF->FindLocalNonAliasTarget(inject->Target);
      if (!target) {
        return false;
      }
      target->SetProperty(inject->Property, value);
    }

    this->GG->CreateGenerationObjects();
    if (this->GG->GetLocalGenerators().empty()) {
      return false;
    }
    this->LG = this->GG->GetLocalGenerators().front().get();
    return true;
  }

  cmGeneratorTarget const* Find(char const* name) const
  {
    return name ? this->LG->FindGeneratorTargetToUse(name) : nullptr;
  }
};

} // namespace

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
  (void)argc;
  (void)argv;

  char tmpl[] = "/tmp/cmake_fuzz_genex_eval_XXXXXX";
  char* dir = mkdtemp(tmpl);
  g_srcDir = dir ? dir : "/tmp/cmake_fuzz_genex_eval";
  g_binDir = g_srcDir + "/build";
  cmSystemTools::MakeDirectory(g_srcDir + "/include");
  cmSystemTools::MakeDirectory(g_binDir);
  WriteFile(g_srcDir + "/fuzz.c", "int fuzz_c(void) { return 0; }\n");
  WriteFile(g_srcDir + "/fuzz.cpp", "int main() { return 0; }\n");
  WriteFile(g_srcDir + "/fuzz.h", "#pragma once\n");

  // A broken fixture would make every input a no-op, so fail loudly here
  // while diagnostics are still printed.
  {
    Fixture fx;
    bool ok = fx.Build(nullptr, std::string()) &&
      !cmSystemTools::GetFatalErrorOccurred();
    for (char const* name : kFixtureTargets) {
      if (ok && !fx.Find(name)) {
        fprintf(stderr, "fixture target missing: %s\n", name);
        ok = false;
      }
    }
    if (!ok) {
      fprintf(stderr, "cmGeneratorExpressionEvalFuzzer: fixture failed\n");
      abort();
    }
  }
  cmSystemTools::ResetErrorOccurredFlag();

  cmSystemTools::SetMessageCallback(
    [](std::string const&, cmMessageMetadata const&) {});
  cmSystemTools::SetStdoutCallback([](std::string const&) {});
  cmSystemTools::SetStderrCallback([](std::string const&) {});

  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size)
{
  if (size < kHeaderSize || size > kHeaderSize + kMaxExprSize) {
    return 0;
  }

  uint8_t const sel0 = data[0];
  uint8_t const sel1 = data[1];
  Mode const mode = static_cast<Mode>(sel0 & 0x3u);
  char const* const headName = kTargets[(sel0 >> 2) & 0x7u];
  std::string const config = kConfigs[(sel0 >> 5) & 0x3u];
  std::string const otherConfig = kConfigs[((sel0 >> 5) + 1) & 0x3u];
  bool const forBuildsystem = (sel0 & 0x80u) != 0;
  std::string const language = kLanguages[sel1 & 0x7u];
  unsigned const propIndex = (sel1 >> 3) & 0x3u;
  char const* const currentName = kTargets[(sel1 >> 5) & 0x7u];

  std::string expr(reinterpret_cast<char const*>(data + kHeaderSize),
                   size - kHeaderSize);

  // The fatal-error flag is global and would stop the next fixture's
  // listfile from running.
  cmSystemTools::ResetErrorOccurredFlag();
  {
    Fixture fx;
    Injection const* inject =
      mode == Mode::Injection ? &kInjections[propIndex] : nullptr;
    if (fx.Build(inject, expr)) {
      cmGeneratorTarget const* head = fx.Find(headName);

      switch (mode) {
        case Mode::Evaluate: {
          (void)cmGeneratorExpression::Evaluate(
            std::move(expr), fx.LG, config, head, nullptr, nullptr, language);
        } break;

        case Mode::Compiled: {
          cmGeneratorExpression ge(fx.CMake);
          std::unique_ptr<cmCompiledGeneratorExpression> cge =
            ge.Parse(std::move(expr));
          cge->SetEvaluateForBuildsystem(forBuildsystem);
          cmGeneratorTarget const* current = fx.Find(currentName);

          cm::GenEx::Context ctx(fx.LG, config, language);
          (void)cge->Evaluate(ctx, nullptr, head, current);
          (void)cge->GetTargets();
          (void)cge->GetAllTargetsSeen();
          (void)cge->GetSeenTargetProperties();
          (void)cge->GetHadContextSensitiveCondition();
          (void)cge->GetHadHeadSensitiveCondition();
          (void)cge->GetHadLinkLanguageSensitiveCondition();
          (void)cge->GetSourceSensitiveTargets();
          if (head) {
            std::map<std::string, std::string> standards;
            cge->GetMaxLanguageStandard(head, standards);
          }

          cm::GenEx::Context otherCtx(fx.LG, otherConfig, language);
          (void)cge->Evaluate(otherCtx, nullptr, head, current);
        } break;

        case Mode::Interpreter: {
          if (!head) {
            head = fx.Find("fuzz_exe");
          }
          cmGeneratorExpressionInterpreter interpreter(fx.LG, config, head,
                                                       language);
          (void)interpreter.Evaluate(std::move(expr),
                                     kInterpreterProperties[propIndex]);
        } break;

        case Mode::Injection: {
          Injection const& inj = kInjections[propIndex];
          (void)cmGeneratorExpression::Evaluate(inj.Reader, fx.LG, config,
                                                fx.Find("fuzz_exe"), nullptr,
                                                nullptr, language);
          std::string direct = "$<TARGET_PROPERTY:";
          direct += inj.Target;
          direct += ',';
          direct += inj.Property;
          direct += '>';
          (void)cmGeneratorExpression::Evaluate(std::move(direct), fx.LG,
                                                config);
        } break;
      }
    }
  }
  cmSystemTools::ResetErrorOccurredFlag();

  return 0;
}
