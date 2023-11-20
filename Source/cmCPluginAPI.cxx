/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
/*
   this file contains the implementation of the C API to CMake. Generally
   these routines just manipulate arguments and then call the associated
   methods on the CMake classes. */

#include "cmCPluginAPI.h"

#include <cstdlib>

#include "cmExecutionStatus.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmState.h"
#include "cmValue.h"
#include "cmVersion.h"

#ifdef __QNX__
#  include <malloc.h> /* for malloc/free on QNX */
#endif

extern "C" {

static void CCONV* cmGetClientData(void* info)
{
  return ((cmLoadedCommandInfo*)info)->ClientData;
}

static void CCONV cmSetClientData(void* info, void* cd)
{
  ((cmLoadedCommandInfo*)info)->ClientData = cd;
}

static void CCONV cmSetError(void* info, const char* err)
{
  if (((cmLoadedCommandInfo*)info)->Error) {
    free(((cmLoadedCommandInfo*)info)->Error);
  }
  ((cmLoadedCommandInfo*)info)->Error = strdup(err);
}

static unsigned int CCONV cmGetCacheMajorVersion(void* arg)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  cmState* state = mf->GetState();
  return state->GetCacheMajorVersion();
}
static unsigned int CCONV cmGetCacheMinorVersion(void* arg)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  cmState* state = mf->GetState();
  return state->GetCacheMinorVersion();
}

static unsigned int CCONV cmGetMajorVersion(void*)
{
  return cmVersion::GetMajorVersion();
}

static unsigned int CCONV cmGetMinorVersion(void*)
{
  return cmVersion::GetMinorVersion();
}

static void CCONV cmAddDefinition(void* arg, const char* name,
                                  const char* value)
{
  if (value) {
    cmMakefile* mf = static_cast<cmMakefile*>(arg);
    mf->AddDefinition(name, value);
  }
}

/* Add a definition to this makefile and the global cmake cache. */
static void CCONV cmAddCacheDefinition(void* arg, const char* name,
                                       const char* value, const char* doc,
                                       int type)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  std::string valueString;
  std::string docString;
  cmValue v;
  cmValue d;
  if (value != nullptr) {
    valueString = value;
    v = cmValue{ valueString };
  }
  if (doc != nullptr) {
    docString = doc;
    d = cmValue{ docString };
  }

  switch (type) {
    case CM_CACHE_BOOL:
      mf->AddCacheDefinition(name, v, d, cmStateEnums::BOOL);
      break;
    case CM_CACHE_PATH:
      mf->AddCacheDefinition(name, v, d, cmStateEnums::PATH);
      break;
    case CM_CACHE_FILEPATH:
      mf->AddCacheDefinition(name, v, d, cmStateEnums::FILEPATH);
      break;
    case CM_CACHE_STRING:
      mf->AddCacheDefinition(name, v, d, cmStateEnums::STRING);
      break;
    case CM_CACHE_INTERNAL:
      mf->AddCacheDefinition(name, v, d, cmStateEnums::INTERNAL);
      break;
    case CM_CACHE_STATIC:
      mf->AddCacheDefinition(name, v, d, cmStateEnums::STATIC);
      break;
  }
}

static const char* CCONV cmGetProjectName(void* arg)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  static std::string name;
  name = mf->GetStateSnapshot().GetProjectName();
  return name.c_str();
}

static const char* CCONV cmGetHomeDirectory(void* arg)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  return mf->GetHomeDirectory().c_str();
}
static const char* CCONV cmGetHomeOutputDirectory(void* arg)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  return mf->GetHomeOutputDirectory().c_str();
}
static const char* CCONV cmGetStartDirectory(void* arg)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  return mf->GetCurrentSourceDirectory().c_str();
}
static const char* CCONV cmGetStartOutputDirectory(void* arg)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  return mf->GetCurrentBinaryDirectory().c_str();
}
static const char* CCONV cmGetCurrentDirectory(void* arg)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  return mf->GetCurrentSourceDirectory().c_str();
}
static const char* CCONV cmGetCurrentOutputDirectory(void* arg)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  return mf->GetCurrentBinaryDirectory().c_str();
}
static const char* CCONV cmGetDefinition(void* arg, const char* def)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  return mf->GetDefinition(def).GetCStr();
}

static int CCONV cmIsOn(void* arg, const char* name)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  return static_cast<int>(mf->IsOn(name));
}

/** Check if a command exists. */
static int CCONV cmCommandExists(void* arg, const char* name)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  return static_cast<int>(mf->GetState()->GetCommand(name) ? 1 : 0);
}

static void CCONV cmAddDefineFlag(void* arg, const char* definition)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  mf->AddDefineFlag(definition);
}

static void CCONV cmAddLinkDirectoryForTarget(void* arg, const char* tgt,
                                              const char* d)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  cmTarget* t = mf->FindLocalNonAliasTarget(tgt);
  if (!t) {
    cmSystemTools::Error(
      "Attempt to add link directories to non-existent target: " +
      std::string(tgt) + " for directory " + std::string(d));
    return;
  }
  t->InsertLinkDirectory(BT<std::string>(d, mf->GetBacktrace()));
}

static void CCONV cmAddExecutable(void* arg, const char* exename, int numSrcs,
                                  const char** srcs, int win32)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  std::vector<std::string> srcs2;
  int i;
  for (i = 0; i < numSrcs; ++i) {
    srcs2.emplace_back(srcs[i]);
  }
  cmTarget* tg = mf->AddExecutable(exename, srcs2);
  if (win32) {
    tg->SetProperty("WIN32_EXECUTABLE", "ON");
  }
}

static void CCONV cmAddUtilityCommand(void* arg, const char* utilityName,
                                      const char* command,
                                      const char* arguments, int all,
                                      int numDepends, const char** depends,
                                      int, const char**)
{
  // Get the makefile instance.  Perform an extra variable expansion
  // now because the API caller expects it.
  cmMakefile* mf = static_cast<cmMakefile*>(arg);

  // Construct the command line for the command.
  cmCustomCommandLine commandLine;
  std::string expand = command;
  commandLine.push_back(mf->ExpandVariablesInString(expand));
  if (arguments && arguments[0]) {
    // TODO: Parse arguments!
    expand = arguments;
    commandLine.push_back(mf->ExpandVariablesInString(expand));
  }
  cmCustomCommandLines commandLines;
  commandLines.push_back(commandLine);

  // Accumulate the list of dependencies.
  std::vector<std::string> depends2;
  for (int i = 0; i < numDepends; ++i) {
    expand = depends[i];
    depends2.push_back(mf->ExpandVariablesInString(expand));
  }

  // Pass the call to the makefile instance.
  auto cc = cm::make_unique<cmCustomCommand>();
  cc->SetDepends(depends2);
  cc->SetCommandLines(commandLines);
  mf->AddUtilityCommand(utilityName, !all, std::move(cc));
}

static void CCONV cmAddCustomCommand(void* arg, const char* source,
                                     const char* command, int numArgs,
                                     const char** args, int numDepends,
                                     const char** depends, int numOutputs,
                                     const char** outputs, const char* target)
{
  // Get the makefile instance.  Perform an extra variable expansion
  // now because the API caller expects it.
  cmMakefile* mf = static_cast<cmMakefile*>(arg);

  // Construct the command line for the command.
  cmCustomCommandLine commandLine;
  std::string expand = command;
  commandLine.push_back(mf->ExpandVariablesInString(expand));
  for (int i = 0; i < numArgs; ++i) {
    expand = args[i];
    commandLine.push_back(mf->ExpandVariablesInString(expand));
  }
  cmCustomCommandLines commandLines;
  commandLines.push_back(commandLine);

  // Accumulate the list of dependencies.
  std::vector<std::string> depends2;
  for (int i = 0; i < numDepends; ++i) {
    expand = depends[i];
    depends2.push_back(mf->ExpandVariablesInString(expand));
  }

  // Accumulate the list of outputs.
  std::vector<std::string> outputs2;
  for (int i = 0; i < numOutputs; ++i) {
    expand = outputs[i];
    outputs2.push_back(mf->ExpandVariablesInString(expand));
  }

  // Pass the call to the makefile instance.
  const char* no_comment = nullptr;
  mf->AddCustomCommandOldStyle(target, outputs2, depends2, source,
                               commandLines, no_comment);
}

static void CCONV cmAddCustomCommandToOutput(void* arg, const char* output,
                                             const char* command, int numArgs,
                                             const char** args,
                                             const char* main_dependency,
                                             int numDepends,
                                             const char** depends)
{
  // Get the makefile instance.  Perform an extra variable expansion
  // now because the API caller expects it.
  cmMakefile* mf = static_cast<cmMakefile*>(arg);

  // Construct the command line for the command.
  cmCustomCommandLine commandLine;
  std::string expand = command;
  commandLine.push_back(mf->ExpandVariablesInString(expand));
  for (int i = 0; i < numArgs; ++i) {
    expand = args[i];
    commandLine.push_back(mf->ExpandVariablesInString(expand));
  }
  cmCustomCommandLines commandLines;
  commandLines.push_back(commandLine);

  // Accumulate the list of dependencies.
  std::vector<std::string> depends2;
  for (int i = 0; i < numDepends; ++i) {
    expand = depends[i];
    depends2.push_back(mf->ExpandVariablesInString(expand));
  }

  // Pass the call to the makefile instance.
  auto cc = cm::make_unique<cmCustomCommand>();
  cc->SetOutputs(output);
  cc->SetMainDependency(main_dependency);
  cc->SetDepends(depends2);
  cc->SetCommandLines(commandLines);
  mf->AddCustomCommandToOutput(std::move(cc));
}

static void CCONV cmAddCustomCommandToTarget(void* arg, const char* target,
                                             const char* command, int numArgs,
                                             const char** args,
                                             int commandType)
{
  // Get the makefile instance.
  cmMakefile* mf = static_cast<cmMakefile*>(arg);

  // Construct the command line for the command.  Perform an extra
  // variable expansion now because the API caller expects it.
  cmCustomCommandLine commandLine;
  std::string expand = command;
  commandLine.push_back(mf->ExpandVariablesInString(expand));
  for (int i = 0; i < numArgs; ++i) {
    expand = args[i];
    commandLine.push_back(mf->ExpandVariablesInString(expand));
  }
  cmCustomCommandLines commandLines;
  commandLines.push_back(commandLine);

  // Select the command type.
  cmCustomCommandType cctype = cmCustomCommandType::POST_BUILD;
  switch (commandType) {
    case CM_PRE_BUILD:
      cctype = cmCustomCommandType::PRE_BUILD;
      break;
    case CM_PRE_LINK:
      cctype = cmCustomCommandType::PRE_LINK;
      break;
    case CM_POST_BUILD:
      cctype = cmCustomCommandType::POST_BUILD;
      break;
  }

  // Pass the call to the makefile instance.
  auto cc = cm::make_unique<cmCustomCommand>();
  cc->SetCommandLines(commandLines);
  mf->AddCustomCommandToTarget(target, cctype, std::move(cc));
}

static void addLinkLibrary(cmMakefile* mf, std::string const& target,
                           std::string const& lib, cmTargetLinkLibraryType llt)
{
  cmTarget* t = mf->FindLocalNonAliasTarget(target);
  if (!t) {
    std::ostringstream e;
    e << "Attempt to add link library \"" << lib << "\" to target \"" << target
      << "\" which is not built in this directory.";
    mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }

  cmTarget* tgt = mf->GetGlobalGenerator()->FindTarget(lib);
  if (tgt && (tgt->GetType() != cmStateEnums::STATIC_LIBRARY) &&
      (tgt->GetType() != cmStateEnums::SHARED_LIBRARY) &&
      (tgt->GetType() != cmStateEnums::INTERFACE_LIBRARY) &&
      !tgt->IsExecutableWithExports()) {
    std::ostringstream e;
    e << "Target \"" << lib << "\" of type "
      << cmState::GetTargetTypeName(tgt->GetType())
      << " may not be linked into another target.  "
      << "One may link only to STATIC or SHARED libraries, or "
      << "to executables with the ENABLE_EXPORTS property set.";
    mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
  }

  t->AddLinkLibrary(*mf, lib, llt);
}

static void CCONV cmAddLinkLibraryForTarget(void* arg, const char* tgt,
                                            const char* value, int libtype)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);

  switch (libtype) {
    case CM_LIBRARY_GENERAL:
      addLinkLibrary(mf, tgt, value, GENERAL_LibraryType);
      break;
    case CM_LIBRARY_DEBUG:
      addLinkLibrary(mf, tgt, value, DEBUG_LibraryType);
      break;
    case CM_LIBRARY_OPTIMIZED:
      addLinkLibrary(mf, tgt, value, OPTIMIZED_LibraryType);
      break;
  }
}

static void CCONV cmAddLibrary(void* arg, const char* libname, int shared,
                               int numSrcs, const char** srcs)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  std::vector<std::string> srcs2;
  int i;
  for (i = 0; i < numSrcs; ++i) {
    srcs2.emplace_back(srcs[i]);
  }
  mf->AddLibrary(
    libname,
    (shared ? cmStateEnums::SHARED_LIBRARY : cmStateEnums::STATIC_LIBRARY),
    srcs2);
}

static char CCONV* cmExpandVariablesInString(void* arg, const char* source,
                                             int escapeQuotes, int atOnly)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  std::string barf = source;
  std::string const& result =
    mf->ExpandVariablesInString(barf, escapeQuotes != 0, atOnly != 0);
  return strdup(result.c_str());
}

static int CCONV cmExecuteCommand(void* arg, const char* name, int numArgs,
                                  const char** args)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);

  std::vector<cmListFileArgument> lffArgs;
  lffArgs.reserve(numArgs);
  for (int i = 0; i < numArgs; ++i) {
    // Assume all arguments are quoted.
    lffArgs.emplace_back(args[i], cmListFileArgument::Quoted, 0);
  }

  cmListFileFunction lff{ name, 0, 0, std::move(lffArgs) };
  cmExecutionStatus status(*mf);
  return mf->ExecuteCommand(lff, status);
}

static void CCONV cmExpandSourceListArguments(void* arg, int numArgs,
                                              const char** args, int* resArgc,
                                              char*** resArgv,
                                              unsigned int startArgumentIndex)
{
  (void)arg;
  (void)startArgumentIndex;
  std::vector<std::string> result;
  int i;
  for (i = 0; i < numArgs; ++i) {
    result.emplace_back(args[i]);
  }
  int resargc = static_cast<int>(result.size());
  char** resargv = nullptr;
  if (resargc) {
    resargv = (char**)malloc(resargc * sizeof(char*));
  }
  for (i = 0; i < resargc; ++i) {
    resargv[i] = strdup(result[i].c_str());
  }
  *resArgc = resargc;
  *resArgv = resargv;
}

static void CCONV cmFreeArguments(int argc, char** argv)
{
  int i;
  for (i = 0; i < argc; ++i) {
    free(argv[i]);
  }
  free(argv);
}

static int CCONV cmGetTotalArgumentSize(int argc, char** argv)
{
  int i;
  int result = 0;
  for (i = 0; i < argc; ++i) {
    if (argv[i]) {
      result = result + static_cast<int>(strlen(argv[i]));
    }
  }
  return result;
}

// Source file proxy object to support the old cmSourceFile/cmMakefile
// API for source files.
struct cmCPluginAPISourceFile
{
  cmSourceFile* RealSourceFile = nullptr;
  std::string SourceName;
  std::string SourceExtension;
  std::string FullPath;
  std::vector<std::string> Depends;
  cmPropertyMap Properties;
};

// Keep a map from real cmSourceFile instances stored in a makefile to
// the CPluginAPI proxy source file.
using cmCPluginAPISourceFileMap =
  std::map<cmSourceFile*, std::unique_ptr<cmCPluginAPISourceFile>>;
static cmCPluginAPISourceFileMap cmCPluginAPISourceFiles;

static void* CCONV cmCreateSourceFile()
{
  return new cmCPluginAPISourceFile;
}

static void* CCONV cmCreateNewSourceFile(void*)
{
  return new cmCPluginAPISourceFile;
}

static void CCONV cmDestroySourceFile(void* arg)
{
  cmCPluginAPISourceFile* sf = static_cast<cmCPluginAPISourceFile*>(arg);
  // Only delete if it was created by cmCreateSourceFile or
  // cmCreateNewSourceFile and is therefore not in the map.
  if (!sf->RealSourceFile) {
    delete sf;
  }
}

static void CCONV* cmGetSource(void* arg, const char* name)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  if (cmSourceFile* rsf = mf->GetSource(name)) {
    // Lookup the proxy source file object for this source.
    auto i = cmCPluginAPISourceFiles.find(rsf);
    if (i == cmCPluginAPISourceFiles.end()) {
      // Create a proxy source file object for this source.
      auto sf = cm::make_unique<cmCPluginAPISourceFile>();
      sf->RealSourceFile = rsf;
      sf->FullPath = rsf->ResolveFullPath();
      sf->SourceName =
        cmSystemTools::GetFilenameWithoutLastExtension(sf->FullPath);
      sf->SourceExtension =
        cmSystemTools::GetFilenameLastExtension(sf->FullPath);

      // Store the proxy in the map so it can be reused and deleted later.
      i = cmCPluginAPISourceFiles.emplace(rsf, std::move(sf)).first;
    }
    return i->second.get();
  }
  return nullptr;
}

static void* CCONV cmAddSource(void* arg, void* arg2)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  cmCPluginAPISourceFile* osf = static_cast<cmCPluginAPISourceFile*>(arg2);
  if (osf->FullPath.empty()) {
    return nullptr;
  }

  // Create the real cmSourceFile instance and copy over saved information.
  cmSourceFile* rsf = mf->GetOrCreateSource(osf->FullPath);
  rsf->SetProperties(osf->Properties);
  // In case the properties contain the GENERATED property,
  // mark the real cmSourceFile as generated.
  if (rsf->GetIsGenerated()) {
    rsf->MarkAsGenerated();
  }
  for (std::string const& d : osf->Depends) {
    rsf->AddDepend(d);
  }

  // Create the proxy for the real source file.
  auto sf = cm::make_unique<cmCPluginAPISourceFile>();
  sf->RealSourceFile = rsf;
  sf->FullPath = osf->FullPath;
  sf->SourceName = osf->SourceName;
  sf->SourceExtension = osf->SourceExtension;

  // Store the proxy in the map so it can be reused and deleted later.
  auto* value = sf.get();
  cmCPluginAPISourceFiles[rsf] = std::move(sf);
  return value;
}

static const char* CCONV cmSourceFileGetSourceName(void* arg)
{
  cmCPluginAPISourceFile* sf = static_cast<cmCPluginAPISourceFile*>(arg);
  return sf->SourceName.c_str();
}

static const char* CCONV cmSourceFileGetFullPath(void* arg)
{
  cmCPluginAPISourceFile* sf = static_cast<cmCPluginAPISourceFile*>(arg);
  return sf->FullPath.c_str();
}

static const char* CCONV cmSourceFileGetProperty(void* arg, const char* prop)
{
  cmCPluginAPISourceFile* sf = static_cast<cmCPluginAPISourceFile*>(arg);
  if (cmSourceFile* rsf = sf->RealSourceFile) {
    return rsf->GetProperty(prop).GetCStr();
  }
  if (!strcmp(prop, "LOCATION")) {
    return sf->FullPath.c_str();
  }
  return sf->Properties.GetPropertyValue(prop).GetCStr();
}

static int CCONV cmSourceFileGetPropertyAsBool(void* arg, const char* prop)
{
  cmCPluginAPISourceFile* sf = static_cast<cmCPluginAPISourceFile*>(arg);
  if (cmSourceFile* rsf = sf->RealSourceFile) {
    return rsf->GetPropertyAsBool(prop) ? 1 : 0;
  }
  return cmIsOn(cmSourceFileGetProperty(arg, prop)) ? 1 : 0;
}

static void CCONV cmSourceFileSetProperty(void* arg, const char* prop,
                                          const char* value)
{
  cmCPluginAPISourceFile* sf = static_cast<cmCPluginAPISourceFile*>(arg);
  if (cmSourceFile* rsf = sf->RealSourceFile) {
    if (!value) {
      rsf->RemoveProperty(prop);
    } else {
      rsf->SetProperty(prop, value);
    }
  } else if (prop) {
    if (!value) {
      value = "NOTFOUND";
    }
    sf->Properties.SetProperty(prop, value);
  }
}

static void CCONV cmSourceFileAddDepend(void* arg, const char* depend)
{
  cmCPluginAPISourceFile* sf = static_cast<cmCPluginAPISourceFile*>(arg);
  if (cmSourceFile* rsf = sf->RealSourceFile) {
    rsf->AddDepend(depend);
  } else {
    sf->Depends.emplace_back(depend);
  }
}

static void CCONV cmSourceFileSetName(void* arg, const char* name,
                                      const char* dir, int numSourceExtensions,
                                      const char** sourceExtensions,
                                      int numHeaderExtensions,
                                      const char** headerExtensions)
{
  cmCPluginAPISourceFile* sf = static_cast<cmCPluginAPISourceFile*>(arg);
  if (sf->RealSourceFile) {
    // SetName is allowed only on temporary source files created by
    // the command for building and passing to AddSource.
    return;
  }
  std::vector<std::string> sourceExts;
  std::vector<std::string> headerExts;
  int i;
  for (i = 0; i < numSourceExtensions; ++i) {
    sourceExts.emplace_back(sourceExtensions[i]);
  }
  for (i = 0; i < numHeaderExtensions; ++i) {
    headerExts.emplace_back(headerExtensions[i]);
  }

  // Save the original name given.
  sf->SourceName = name;

  // Convert the name to a full path in case the given name is a
  // relative path.
  std::string pathname = cmSystemTools::CollapseFullPath(name, dir);

  // First try and see whether the listed file can be found
  // as is without extensions added on.
  std::string hname = pathname;
  if (cmSystemTools::FileExists(hname)) {
    sf->SourceName = cmSystemTools::GetFilenamePath(name);
    if (!sf->SourceName.empty()) {
      sf->SourceName += "/";
    }
    sf->SourceName += cmSystemTools::GetFilenameWithoutLastExtension(name);
    std::string::size_type pos = hname.rfind('.');
    if (pos != std::string::npos) {
      sf->SourceExtension = hname.substr(pos + 1, hname.size() - pos);
      if (cmSystemTools::FileIsFullPath(name)) {
        std::string::size_type pos2 = hname.rfind('/');
        if (pos2 != std::string::npos) {
          sf->SourceName = hname.substr(pos2 + 1, pos - pos2 - 1);
        }
      }
    }

    sf->FullPath = hname;
    return;
  }

  // Next, try the various source extensions
  for (std::string const& ext : sourceExts) {
    hname = cmStrCat(pathname, '.', ext);
    if (cmSystemTools::FileExists(hname)) {
      sf->SourceExtension = ext;
      sf->FullPath = hname;
      return;
    }
  }

  // Finally, try the various header extensions
  for (std::string const& ext : headerExts) {
    hname = cmStrCat(pathname, '.', ext);
    if (cmSystemTools::FileExists(hname)) {
      sf->SourceExtension = ext;
      sf->FullPath = hname;
      return;
    }
  }

  std::ostringstream e;
  e << "Cannot find source file \"" << pathname << "\"";
  e << "\n\nTried extensions";
  for (std::string const& ext : sourceExts) {
    e << " ." << ext;
  }
  for (std::string const& ext : headerExts) {
    e << " ." << ext;
  }
  cmSystemTools::Error(e.str());
}

static void CCONV cmSourceFileSetName2(void* arg, const char* name,
                                       const char* dir, const char* ext,
                                       int headerFileOnly)
{
  cmCPluginAPISourceFile* sf = static_cast<cmCPluginAPISourceFile*>(arg);
  if (sf->RealSourceFile) {
    // SetName is allowed only on temporary source files created by
    // the command for building and passing to AddSource.
    return;
  }

  // Implement the old SetName method code here.
  if (headerFileOnly) {
    sf->Properties.SetProperty("HEADER_FILE_ONLY", "1");
  }
  sf->SourceName = name;
  std::string fname = sf->SourceName;
  if (cmNonempty(ext)) {
    fname += ".";
    fname += ext;
  }
  sf->FullPath = cmSystemTools::CollapseFullPath(fname, dir);
  cmSystemTools::ConvertToUnixSlashes(sf->FullPath);
  sf->SourceExtension = ext;
}

static char* CCONV cmGetFilenameWithoutExtension(const char* name)
{
  std::string sres = cmSystemTools::GetFilenameWithoutExtension(name);
  return strdup(sres.c_str());
}

static char* CCONV cmGetFilenamePath(const char* name)
{
  std::string sres = cmSystemTools::GetFilenamePath(name);
  return strdup(sres.c_str());
}

static char* CCONV cmCapitalized(const char* name)
{
  std::string sres = cmSystemTools::Capitalized(name);
  return strdup(sres.c_str());
}

static void CCONV cmCopyFileIfDifferent(const char* name1, const char* name2)
{
  cmSystemTools::CopyFileIfDifferent(name1, name2);
}

static void CCONV cmRemoveFile(const char* name)
{
  cmSystemTools::RemoveFile(name);
}

static void CCONV cmDisplayStatus(void* arg, const char* message)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  mf->DisplayStatus(message, -1);
}

static void CCONV cmFree(void* data)
{
  free(data);
}

static void CCONV DefineSourceFileProperty(void* arg, const char* name,
                                           const char* briefDocs,
                                           const char* longDocs, int chained)
{
  cmMakefile* mf = static_cast<cmMakefile*>(arg);
  mf->GetState()->DefineProperty(name, cmProperty::SOURCE_FILE,
                                 briefDocs ? briefDocs : "",
                                 longDocs ? longDocs : "", chained != 0);
}

} // close the extern "C" scope

static cmCAPI cmStaticCAPI = {
  cmGetClientData,
  cmGetTotalArgumentSize,
  cmFreeArguments,
  cmSetClientData,
  cmSetError,
  cmAddCacheDefinition,
  cmAddCustomCommand,
  cmAddDefineFlag,
  cmAddDefinition,
  cmAddExecutable,
  cmAddLibrary,
  cmAddLinkDirectoryForTarget,
  cmAddLinkLibraryForTarget,
  cmAddUtilityCommand,
  cmCommandExists,
  cmExecuteCommand,
  cmExpandSourceListArguments,
  cmExpandVariablesInString,
  cmGetCacheMajorVersion,
  cmGetCacheMinorVersion,
  cmGetCurrentDirectory,
  cmGetCurrentOutputDirectory,
  cmGetDefinition,
  cmGetHomeDirectory,
  cmGetHomeOutputDirectory,
  cmGetMajorVersion,
  cmGetMinorVersion,
  cmGetProjectName,
  cmGetStartDirectory,
  cmGetStartOutputDirectory,
  cmIsOn,

  cmAddSource,
  cmCreateSourceFile,
  cmDestroySourceFile,
  cmGetSource,
  cmSourceFileAddDepend,
  cmSourceFileGetProperty,
  cmSourceFileGetPropertyAsBool,
  cmSourceFileGetSourceName,
  cmSourceFileGetFullPath,
  cmSourceFileSetName,
  cmSourceFileSetName2,
  cmSourceFileSetProperty,

  cmCapitalized,
  cmCopyFileIfDifferent,
  cmGetFilenameWithoutExtension,
  cmGetFilenamePath,
  cmRemoveFile,
  cmFree,

  cmAddCustomCommandToOutput,
  cmAddCustomCommandToTarget,
  cmDisplayStatus,
  cmCreateNewSourceFile,
  DefineSourceFileProperty,
};
