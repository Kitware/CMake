/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#if !defined(_WIN32) && !defined(__sun) && !defined(__OpenBSD__)
// POSIX APIs are needed
// NOLINTNEXTLINE(bugprone-reserved-identifier)
#  define _POSIX_C_SOURCE 200809L
#endif
#if defined(__FreeBSD__) || defined(__NetBSD__)
// For isascii
// NOLINTNEXTLINE(bugprone-reserved-identifier)
#  define _XOPEN_SOURCE 700
#endif

#include "cmLoadCommandCommand.h"

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <cm/memory>

#include "cmCPluginAPI.h"
#include "cmDynamicLoader.h"
#include "cmExecutionStatus.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "cmCPluginAPI.cxx"

#ifdef __QNX__
#  include <malloc.h> /* for malloc/free on QNX */
#endif

#if defined(__clang__) && defined(__has_warning)
#  if __has_warning("-Wcast-function-type-strict")
#    pragma clang diagnostic ignored "-Wcast-function-type-strict"
#  endif
#endif

namespace {

const char* LastName = nullptr;

extern "C" void TrapsForSignals(int sig);
extern "C" void TrapsForSignals(int sig)
{
  fprintf(stderr, "CMake loaded command %s crashed with signal: %d.\n",
          LastName, sig);
}

struct SignalHandlerGuard
{
  explicit SignalHandlerGuard(const char* name)
  {
    LastName = name ? name : "????";

    signal(SIGSEGV, TrapsForSignals);
#ifdef SIGBUS
    signal(SIGBUS, TrapsForSignals);
#endif
    signal(SIGILL, TrapsForSignals);
  }

  ~SignalHandlerGuard()
  {
    signal(SIGSEGV, nullptr);
#ifdef SIGBUS
    signal(SIGBUS, nullptr);
#endif
    signal(SIGILL, nullptr);
  }

  SignalHandlerGuard(SignalHandlerGuard const&) = delete;
  SignalHandlerGuard& operator=(SignalHandlerGuard const&) = delete;
};

struct LoadedCommandImpl : cmLoadedCommandInfo
{
  explicit LoadedCommandImpl(CM_INIT_FUNCTION init)
    : cmLoadedCommandInfo{ 0,       0,       &cmStaticCAPI, 0,
                           nullptr, nullptr, nullptr,       nullptr,
                           nullptr, nullptr, nullptr,       nullptr }
  {
    init(this);
  }

  ~LoadedCommandImpl()
  {
    if (this->Destructor) {
      SignalHandlerGuard guard(this->Name);
#if defined(__NVCOMPILER) || defined(__LCC__)
      static_cast<void>(guard); // convince compiler var is used
#endif
      this->Destructor(this);
    }
    if (this->Error) {
      free(this->Error);
    }
  }

  LoadedCommandImpl(LoadedCommandImpl const&) = delete;
  LoadedCommandImpl& operator=(LoadedCommandImpl const&) = delete;

  int DoInitialPass(cmMakefile* mf, int argc, char* argv[])
  {
    SignalHandlerGuard guard(this->Name);
#if defined(__NVCOMPILER) || defined(__LCC__)
    static_cast<void>(guard); // convince compiler var is used
#endif
    return this->InitialPass(this, mf, argc, argv);
  }

  void DoFinalPass(cmMakefile* mf)
  {
    SignalHandlerGuard guard(this->Name);
#if defined(__NVCOMPILER) || defined(__LCC__)
    static_cast<void>(guard); // convince compiler var is used
#endif
    this->FinalPass(this, mf);
  }
};

// a class for loadabple commands
class cmLoadedCommand
{
public:
  explicit cmLoadedCommand(CM_INIT_FUNCTION init)
    : Impl(std::make_shared<LoadedCommandImpl>(init))
  {
  }

  bool operator()(std::vector<cmListFileArgument> const& args,
                  cmExecutionStatus& status) const;

private:
  std::shared_ptr<LoadedCommandImpl> Impl;
};

bool cmLoadedCommand::operator()(
  std::vector<cmListFileArgument> const& arguments,
  cmExecutionStatus& status) const
{
  cmMakefile& mf = status.GetMakefile();

  std::vector<std::string> args;
  if (!mf.ExpandArguments(arguments, args)) {
    return true;
  }

  if (!this->Impl->InitialPass) {
    return true;
  }

  // clear the error string
  if (this->Impl->Error) {
    free(this->Impl->Error);
  }

  // create argc and argv and then invoke the command
  int argc = static_cast<int>(args.size());
  char** argv = nullptr;
  if (argc) {
    argv = static_cast<char**>(malloc(argc * sizeof(char*)));
  }
  int i;
  for (i = 0; i < argc; ++i) {
    argv[i] = strdup(args[i].c_str());
  }
  int result = this->Impl->DoInitialPass(&mf, argc, argv);
  cmFreeArguments(argc, argv);

  if (result) {
    if (this->Impl->FinalPass) {
      auto impl = this->Impl;
      mf.AddGeneratorAction(
        [impl](cmLocalGenerator& lg, const cmListFileBacktrace&) {
          impl->DoFinalPass(lg.GetMakefile());
        });
    }
    return true;
  }

  /* Initial Pass must have failed so set the error string */
  if (this->Impl->Error) {
    status.SetError(this->Impl->Error);
  }
  return false;
}

} // namespace

// cmLoadCommandCommand
bool cmLoadCommandCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  if (args.empty()) {
    return true;
  }

  // Construct a variable to report what file was loaded, if any.
  // Start by removing the definition in case of failure.
  std::string reportVar = cmStrCat("CMAKE_LOADED_COMMAND_", args[0]);
  status.GetMakefile().RemoveDefinition(reportVar);

  // the file must exist
  std::string moduleName = cmStrCat(
    status.GetMakefile().GetRequiredDefinition("CMAKE_SHARED_MODULE_PREFIX"),
    "cm", args[0],
    status.GetMakefile().GetRequiredDefinition("CMAKE_SHARED_MODULE_SUFFIX"));

  // search for the file
  std::vector<std::string> path;
  for (unsigned int j = 1; j < args.size(); j++) {
    // expand variables
    std::string exp = args[j];
    cmSystemTools::ExpandRegistryValues(exp);

    // Glob the entry in case of wildcards.
    cmSystemTools::GlobDirs(exp, path);
  }

  // Try to find the program.
  std::string fullPath = cmSystemTools::FindFile(moduleName, path);
  if (fullPath.empty()) {
    status.SetError(cmStrCat("Attempt to load command failed from file \"",
                             moduleName, "\""));
    return false;
  }

  // try loading the shared library / dll
  cmsys::DynamicLoader::LibraryHandle lib =
    cmDynamicLoader::OpenLibrary(fullPath.c_str());
  if (!lib) {
    std::string err =
      cmStrCat("Attempt to load the library ", fullPath, " failed.");
    const char* error = cmsys::DynamicLoader::LastError();
    if (error) {
      err += " Additional error info is:\n";
      err += error;
    }
    status.SetError(err);
    return false;
  }

  // Report what file was loaded for this command.
  status.GetMakefile().AddDefinition(reportVar, fullPath);

  // find the init function
  std::string initFuncName = args[0] + "Init";
  CM_INIT_FUNCTION initFunction = reinterpret_cast<CM_INIT_FUNCTION>(
    cmsys::DynamicLoader::GetSymbolAddress(lib, initFuncName));
  if (!initFunction) {
    initFuncName = cmStrCat('_', args[0], "Init");
    initFunction = reinterpret_cast<CM_INIT_FUNCTION>(
      cmsys::DynamicLoader::GetSymbolAddress(lib, initFuncName));
  }
  // if the symbol is found call it to set the name on the
  // function blocker
  if (initFunction) {
    return status.GetMakefile().GetState()->AddScriptedCommand(
      args[0],
      BT<cmState::Command>(cmLoadedCommand(initFunction),
                           status.GetMakefile().GetBacktrace()),
      status.GetMakefile());
  }
  status.SetError("Attempt to load command failed. "
                  "No init function found.");
  return false;
}
