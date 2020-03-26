/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallCommand.h"

#include <cstddef>
#include <set>
#include <sstream>
#include <utility>

#include <cm/memory>

#include "cmsys/Glob.hxx"

#include "cm_static_string_view.hxx"

#include "cmArgumentParser.h"
#include "cmExecutionStatus.h"
#include "cmExportSet.h"
#include "cmGeneratorExpression.h"
#include "cmGlobalGenerator.h"
#include "cmInstallCommandArguments.h"
#include "cmInstallDirectoryGenerator.h"
#include "cmInstallExportGenerator.h"
#include "cmInstallFilesGenerator.h"
#include "cmInstallGenerator.h"
#include "cmInstallScriptGenerator.h"
#include "cmInstallTargetGenerator.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSubcommandTable.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetExport.h"

namespace {

class Helper
{
public:
  Helper(cmExecutionStatus& status)
    : Status(status)
    , Makefile(&status.GetMakefile())
  {
    this->DefaultComponentName = this->Makefile->GetSafeDefinition(
      "CMAKE_INSTALL_DEFAULT_COMPONENT_NAME");
    if (this->DefaultComponentName.empty()) {
      this->DefaultComponentName = "Unspecified";
    }
  }

  void SetError(std::string const& err) { this->Status.SetError(err); }

  bool MakeFilesFullPath(const char* modeName,
                         const std::vector<std::string>& relFiles,
                         std::vector<std::string>& absFiles);
  bool CheckCMP0006(bool& failure);

  std::string GetDestination(const cmInstallCommandArguments* args,
                             const std::string& varName,
                             const std::string& guess);
  std::string GetRuntimeDestination(const cmInstallCommandArguments* args);
  std::string GetSbinDestination(const cmInstallCommandArguments* args);
  std::string GetArchiveDestination(const cmInstallCommandArguments* args);
  std::string GetLibraryDestination(const cmInstallCommandArguments* args);
  std::string GetIncludeDestination(const cmInstallCommandArguments* args);
  std::string GetSysconfDestination(const cmInstallCommandArguments* args);
  std::string GetSharedStateDestination(const cmInstallCommandArguments* args);
  std::string GetLocalStateDestination(const cmInstallCommandArguments* args);
  std::string GetRunStateDestination(const cmInstallCommandArguments* args);
  std::string GetDataRootDestination(const cmInstallCommandArguments* args);
  std::string GetDataDestination(const cmInstallCommandArguments* args);
  std::string GetInfoDestination(const cmInstallCommandArguments* args);
  std::string GetLocaleDestination(const cmInstallCommandArguments* args);
  std::string GetManDestination(const cmInstallCommandArguments* args);
  std::string GetDocDestination(const cmInstallCommandArguments* args);
  std::string GetDestinationForType(const cmInstallCommandArguments* args,
                                    const std::string& type);

  cmExecutionStatus& Status;
  cmMakefile* Makefile;
  std::string DefaultComponentName;
};

std::unique_ptr<cmInstallTargetGenerator> CreateInstallTargetGenerator(
  cmTarget& target, const cmInstallCommandArguments& args, bool impLib,
  cmListFileBacktrace const& backtrace, const std::string& destination,
  bool forceOpt = false, bool namelink = false)
{
  cmInstallGenerator::MessageLevel message =
    cmInstallGenerator::SelectMessageLevel(target.GetMakefile());
  target.SetHaveInstallRule(true);
  const std::string& component =
    namelink ? args.GetNamelinkComponent() : args.GetComponent();
  auto g = cm::make_unique<cmInstallTargetGenerator>(
    target.GetName(), destination, impLib, args.GetPermissions(),
    args.GetConfigurations(), component, message, args.GetExcludeFromAll(),
    args.GetOptional() || forceOpt, backtrace);
  target.AddInstallGenerator(g.get());
  return g;
}

std::unique_ptr<cmInstallTargetGenerator> CreateInstallTargetGenerator(
  cmTarget& target, const cmInstallCommandArguments& args, bool impLib,
  cmListFileBacktrace const& backtrace, bool forceOpt = false,
  bool namelink = false)
{
  return CreateInstallTargetGenerator(target, args, impLib, backtrace,
                                      args.GetDestination(), forceOpt,
                                      namelink);
}

std::unique_ptr<cmInstallFilesGenerator> CreateInstallFilesGenerator(
  cmMakefile* mf, const std::vector<std::string>& absFiles,
  const cmInstallCommandArguments& args, bool programs,
  const std::string& destination)
{
  cmInstallGenerator::MessageLevel message =
    cmInstallGenerator::SelectMessageLevel(mf);
  return cm::make_unique<cmInstallFilesGenerator>(
    absFiles, destination, programs, args.GetPermissions(),
    args.GetConfigurations(), args.GetComponent(), message,
    args.GetExcludeFromAll(), args.GetRename(), args.GetOptional());
}

std::unique_ptr<cmInstallFilesGenerator> CreateInstallFilesGenerator(
  cmMakefile* mf, const std::vector<std::string>& absFiles,
  const cmInstallCommandArguments& args, bool programs)
{
  return CreateInstallFilesGenerator(mf, absFiles, args, programs,
                                     args.GetDestination());
}

std::set<std::string> const allowedTypes{
  "BIN",         "SBIN",       "LIB",      "INCLUDE", "SYSCONF",
  "SHAREDSTATE", "LOCALSTATE", "RUNSTATE", "DATA",    "INFO",
  "LOCALE",      "MAN",        "DOC",
};

bool HandleScriptMode(std::vector<std::string> const& args,
                      cmExecutionStatus& status)
{
  Helper helper(status);

  std::string component = helper.DefaultComponentName;
  int componentCount = 0;
  bool doing_script = false;
  bool doing_code = false;
  bool exclude_from_all = false;

  // Scan the args once for COMPONENT. Only allow one.
  //
  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i] == "COMPONENT" && i + 1 < args.size()) {
      ++componentCount;
      ++i;
      component = args[i];
    }
    if (args[i] == "EXCLUDE_FROM_ALL") {
      exclude_from_all = true;
    }
  }

  if (componentCount > 1) {
    status.SetError("given more than one COMPONENT for the SCRIPT or CODE "
                    "signature of the INSTALL command. "
                    "Use multiple INSTALL commands with one COMPONENT each.");
    return false;
  }

  // Scan the args again, this time adding install generators each time we
  // encounter a SCRIPT or CODE arg:
  //
  for (std::string const& arg : args) {
    if (arg == "SCRIPT") {
      doing_script = true;
      doing_code = false;
    } else if (arg == "CODE") {
      doing_script = false;
      doing_code = true;
    } else if (arg == "COMPONENT") {
      doing_script = false;
      doing_code = false;
    } else if (doing_script) {
      doing_script = false;
      std::string script = arg;
      if (!cmSystemTools::FileIsFullPath(script)) {
        script =
          cmStrCat(helper.Makefile->GetCurrentSourceDirectory(), '/', arg);
      }
      if (cmSystemTools::FileIsDirectory(script)) {
        status.SetError("given a directory as value of SCRIPT argument.");
        return false;
      }
      helper.Makefile->AddInstallGenerator(
        cm::make_unique<cmInstallScriptGenerator>(script, false, component,
                                                  exclude_from_all));
    } else if (doing_code) {
      doing_code = false;
      std::string const& code = arg;
      helper.Makefile->AddInstallGenerator(
        cm::make_unique<cmInstallScriptGenerator>(code, true, component,
                                                  exclude_from_all));
    }
  }

  if (doing_script) {
    status.SetError("given no value for SCRIPT argument.");
    return false;
  }
  if (doing_code) {
    status.SetError("given no value for CODE argument.");
    return false;
  }

  // Tell the global generator about any installation component names
  // specified.
  helper.Makefile->GetGlobalGenerator()->AddInstallComponent(component);

  return true;
}

bool HandleTargetsMode(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  Helper helper(status);

  // This is the TARGETS mode.
  std::vector<cmTarget*> targets;

  struct ArgVectors
  {
    std::vector<std::string> Archive;
    std::vector<std::string> Library;
    std::vector<std::string> Runtime;
    std::vector<std::string> Object;
    std::vector<std::string> Framework;
    std::vector<std::string> Bundle;
    std::vector<std::string> Includes;
    std::vector<std::string> PrivateHeader;
    std::vector<std::string> PublicHeader;
    std::vector<std::string> Resource;
  };

  static auto const argHelper =
    cmArgumentParser<ArgVectors>{}
      .Bind("ARCHIVE"_s, &ArgVectors::Archive)
      .Bind("LIBRARY"_s, &ArgVectors::Library)
      .Bind("RUNTIME"_s, &ArgVectors::Runtime)
      .Bind("OBJECTS"_s, &ArgVectors::Object)
      .Bind("FRAMEWORK"_s, &ArgVectors::Framework)
      .Bind("BUNDLE"_s, &ArgVectors::Bundle)
      .Bind("INCLUDES"_s, &ArgVectors::Includes)
      .Bind("PRIVATE_HEADER"_s, &ArgVectors::PrivateHeader)
      .Bind("PUBLIC_HEADER"_s, &ArgVectors::PublicHeader)
      .Bind("RESOURCE"_s, &ArgVectors::Resource);

  std::vector<std::string> genericArgVector;
  ArgVectors const argVectors = argHelper.Parse(args, &genericArgVector);

  // now parse the generic args (i.e. the ones not specialized on LIBRARY/
  // ARCHIVE, RUNTIME etc. (see above)
  // These generic args also contain the targets and the export stuff
  std::vector<std::string> targetList;
  std::string exports;
  std::vector<std::string> unknownArgs;
  cmInstallCommandArguments genericArgs(helper.DefaultComponentName);
  genericArgs.Bind("TARGETS"_s, targetList);
  genericArgs.Bind("EXPORT"_s, exports);
  genericArgs.Parse(genericArgVector, &unknownArgs);
  bool success = genericArgs.Finalize();

  cmInstallCommandArguments archiveArgs(helper.DefaultComponentName);
  cmInstallCommandArguments libraryArgs(helper.DefaultComponentName);
  cmInstallCommandArguments runtimeArgs(helper.DefaultComponentName);
  cmInstallCommandArguments objectArgs(helper.DefaultComponentName);
  cmInstallCommandArguments frameworkArgs(helper.DefaultComponentName);
  cmInstallCommandArguments bundleArgs(helper.DefaultComponentName);
  cmInstallCommandArguments privateHeaderArgs(helper.DefaultComponentName);
  cmInstallCommandArguments publicHeaderArgs(helper.DefaultComponentName);
  cmInstallCommandArguments resourceArgs(helper.DefaultComponentName);
  cmInstallCommandIncludesArgument includesArgs;

  // now parse the args for specific parts of the target (e.g. LIBRARY,
  // RUNTIME, ARCHIVE etc.
  archiveArgs.Parse(argVectors.Archive, &unknownArgs);
  libraryArgs.Parse(argVectors.Library, &unknownArgs);
  runtimeArgs.Parse(argVectors.Runtime, &unknownArgs);
  objectArgs.Parse(argVectors.Object, &unknownArgs);
  frameworkArgs.Parse(argVectors.Framework, &unknownArgs);
  bundleArgs.Parse(argVectors.Bundle, &unknownArgs);
  privateHeaderArgs.Parse(argVectors.PrivateHeader, &unknownArgs);
  publicHeaderArgs.Parse(argVectors.PublicHeader, &unknownArgs);
  resourceArgs.Parse(argVectors.Resource, &unknownArgs);
  includesArgs.Parse(&argVectors.Includes, &unknownArgs);

  if (!unknownArgs.empty()) {
    // Unknown argument.
    status.SetError(
      cmStrCat("TARGETS given unknown argument \"", unknownArgs[0], "\"."));
    return false;
  }

  // apply generic args
  archiveArgs.SetGenericArguments(&genericArgs);
  libraryArgs.SetGenericArguments(&genericArgs);
  runtimeArgs.SetGenericArguments(&genericArgs);
  objectArgs.SetGenericArguments(&genericArgs);
  frameworkArgs.SetGenericArguments(&genericArgs);
  bundleArgs.SetGenericArguments(&genericArgs);
  privateHeaderArgs.SetGenericArguments(&genericArgs);
  publicHeaderArgs.SetGenericArguments(&genericArgs);
  resourceArgs.SetGenericArguments(&genericArgs);

  success = success && archiveArgs.Finalize();
  success = success && libraryArgs.Finalize();
  success = success && runtimeArgs.Finalize();
  success = success && objectArgs.Finalize();
  success = success && frameworkArgs.Finalize();
  success = success && bundleArgs.Finalize();
  success = success && privateHeaderArgs.Finalize();
  success = success && publicHeaderArgs.Finalize();
  success = success && resourceArgs.Finalize();

  if (!success) {
    return false;
  }

  // Enforce argument rules too complex to specify for the
  // general-purpose parser.
  if (archiveArgs.GetNamelinkOnly() || runtimeArgs.GetNamelinkOnly() ||
      objectArgs.GetNamelinkOnly() || frameworkArgs.GetNamelinkOnly() ||
      bundleArgs.GetNamelinkOnly() || privateHeaderArgs.GetNamelinkOnly() ||
      publicHeaderArgs.GetNamelinkOnly() || resourceArgs.GetNamelinkOnly()) {
    status.SetError(
      "TARGETS given NAMELINK_ONLY option not in LIBRARY group.  "
      "The NAMELINK_ONLY option may be specified only following LIBRARY.");
    return false;
  }
  if (archiveArgs.GetNamelinkSkip() || runtimeArgs.GetNamelinkSkip() ||
      objectArgs.GetNamelinkSkip() || frameworkArgs.GetNamelinkSkip() ||
      bundleArgs.GetNamelinkSkip() || privateHeaderArgs.GetNamelinkSkip() ||
      publicHeaderArgs.GetNamelinkSkip() || resourceArgs.GetNamelinkSkip()) {
    status.SetError(
      "TARGETS given NAMELINK_SKIP option not in LIBRARY group.  "
      "The NAMELINK_SKIP option may be specified only following LIBRARY.");
    return false;
  }
  if (archiveArgs.HasNamelinkComponent() ||
      runtimeArgs.HasNamelinkComponent() ||
      objectArgs.HasNamelinkComponent() ||
      frameworkArgs.HasNamelinkComponent() ||
      bundleArgs.HasNamelinkComponent() ||
      privateHeaderArgs.HasNamelinkComponent() ||
      publicHeaderArgs.HasNamelinkComponent() ||
      resourceArgs.HasNamelinkComponent()) {
    status.SetError(
      "TARGETS given NAMELINK_COMPONENT option not in LIBRARY group.  "
      "The NAMELINK_COMPONENT option may be specified only following "
      "LIBRARY.");
    return false;
  }
  if (libraryArgs.GetNamelinkOnly() && libraryArgs.GetNamelinkSkip()) {
    status.SetError("TARGETS given NAMELINK_ONLY and NAMELINK_SKIP.  "
                    "At most one of these two options may be specified.");
    return false;
  }
  if (!genericArgs.GetType().empty() || !archiveArgs.GetType().empty() ||
      !libraryArgs.GetType().empty() || !runtimeArgs.GetType().empty() ||
      !objectArgs.GetType().empty() || !frameworkArgs.GetType().empty() ||
      !bundleArgs.GetType().empty() || !privateHeaderArgs.GetType().empty() ||
      !publicHeaderArgs.GetType().empty() || !resourceArgs.GetType().empty()) {
    status.SetError(
      "TARGETS given TYPE option. The TYPE option may only be specified in "
      " install(FILES) and install(DIRECTORIES).");
    return false;
  }

  // Select the mode for installing symlinks to versioned shared libraries.
  cmInstallTargetGenerator::NamelinkModeType namelinkMode =
    cmInstallTargetGenerator::NamelinkModeNone;
  if (libraryArgs.GetNamelinkOnly()) {
    namelinkMode = cmInstallTargetGenerator::NamelinkModeOnly;
  } else if (libraryArgs.GetNamelinkSkip()) {
    namelinkMode = cmInstallTargetGenerator::NamelinkModeSkip;
  }

  // Check if there is something to do.
  if (targetList.empty()) {
    return true;
  }

  for (std::string const& tgt : targetList) {

    if (helper.Makefile->IsAlias(tgt)) {
      status.SetError(
        cmStrCat("TARGETS given target \"", tgt, "\" which is an alias."));
      return false;
    }
    // Lookup this target in the current directory.
    cmTarget* target = helper.Makefile->FindLocalNonAliasTarget(tgt);
    if (!target) {
      // If no local target has been found, find it in the global scope.
      cmTarget* const global_target =
        helper.Makefile->GetGlobalGenerator()->FindTarget(tgt, true);
      if (global_target && !global_target->IsImported()) {
        target = global_target;
      }
    }
    if (target) {
      // Found the target.  Check its type.
      if (target->GetType() != cmStateEnums::EXECUTABLE &&
          target->GetType() != cmStateEnums::STATIC_LIBRARY &&
          target->GetType() != cmStateEnums::SHARED_LIBRARY &&
          target->GetType() != cmStateEnums::MODULE_LIBRARY &&
          target->GetType() != cmStateEnums::OBJECT_LIBRARY &&
          target->GetType() != cmStateEnums::INTERFACE_LIBRARY) {
        status.SetError(
          cmStrCat("TARGETS given target \"", tgt,
                   "\" which is not an executable, library, or module."));
        return false;
      }
      // Store the target in the list to be installed.
      targets.push_back(target);
    } else {
      // Did not find the target.
      status.SetError(
        cmStrCat("TARGETS given target \"", tgt, "\" which does not exist."));
      return false;
    }
  }

  // Keep track of whether we will be performing an installation of
  // any files of the given type.
  bool installsArchive = false;
  bool installsLibrary = false;
  bool installsNamelink = false;
  bool installsRuntime = false;
  bool installsObject = false;
  bool installsFramework = false;
  bool installsBundle = false;
  bool installsPrivateHeader = false;
  bool installsPublicHeader = false;
  bool installsResource = false;

  // Generate install script code to install the given targets.
  for (cmTarget* ti : targets) {
    // Handle each target type.
    cmTarget& target = *ti;
    std::unique_ptr<cmInstallTargetGenerator> archiveGenerator;
    std::unique_ptr<cmInstallTargetGenerator> libraryGenerator;
    std::unique_ptr<cmInstallTargetGenerator> namelinkGenerator;
    std::unique_ptr<cmInstallTargetGenerator> runtimeGenerator;
    std::unique_ptr<cmInstallTargetGenerator> objectGenerator;
    std::unique_ptr<cmInstallTargetGenerator> frameworkGenerator;
    std::unique_ptr<cmInstallTargetGenerator> bundleGenerator;
    std::unique_ptr<cmInstallFilesGenerator> privateHeaderGenerator;
    std::unique_ptr<cmInstallFilesGenerator> publicHeaderGenerator;
    std::unique_ptr<cmInstallFilesGenerator> resourceGenerator;

    // Avoid selecting default destinations for PUBLIC_HEADER and
    // PRIVATE_HEADER if any artifacts are specified.
    bool artifactsSpecified = false;

    // Track whether this is a namelink-only rule.
    bool namelinkOnly = false;

    switch (target.GetType()) {
      case cmStateEnums::SHARED_LIBRARY: {
        // Shared libraries are handled differently on DLL and non-DLL
        // platforms.  All windows platforms are DLL platforms including
        // cygwin.  Currently no other platform is a DLL platform.
        if (target.IsDLLPlatform()) {
          // When in namelink only mode skip all libraries on Windows.
          if (namelinkMode == cmInstallTargetGenerator::NamelinkModeOnly) {
            continue;
          }

          // This is a DLL platform.
          if (!archiveArgs.GetDestination().empty()) {
            // The import library uses the ARCHIVE properties.
            archiveGenerator = CreateInstallTargetGenerator(
              target, archiveArgs, true, helper.Makefile->GetBacktrace());
            artifactsSpecified = true;
          }
          if (!runtimeArgs.GetDestination().empty()) {
            // The DLL uses the RUNTIME properties.
            runtimeGenerator = CreateInstallTargetGenerator(
              target, runtimeArgs, false, helper.Makefile->GetBacktrace());
            artifactsSpecified = true;
          }
          if (!archiveGenerator && !runtimeGenerator) {
            archiveGenerator = CreateInstallTargetGenerator(
              target, archiveArgs, true, helper.Makefile->GetBacktrace(),
              helper.GetArchiveDestination(nullptr));
            runtimeGenerator = CreateInstallTargetGenerator(
              target, runtimeArgs, false, helper.Makefile->GetBacktrace(),
              helper.GetRuntimeDestination(nullptr));
          }
        } else {
          // This is a non-DLL platform.
          // If it is marked with FRAMEWORK property use the FRAMEWORK set of
          // INSTALL properties. Otherwise, use the LIBRARY properties.
          if (target.IsFrameworkOnApple()) {
            // When in namelink only mode skip frameworks.
            if (namelinkMode == cmInstallTargetGenerator::NamelinkModeOnly) {
              continue;
            }

            // Use the FRAMEWORK properties.
            if (!frameworkArgs.GetDestination().empty()) {
              frameworkGenerator = CreateInstallTargetGenerator(
                target, frameworkArgs, false, helper.Makefile->GetBacktrace());
            } else {
              status.SetError(
                cmStrCat("TARGETS given no FRAMEWORK DESTINATION for shared "
                         "library FRAMEWORK target \"",
                         target.GetName(), "\"."));
              return false;
            }
          } else {
            // The shared library uses the LIBRARY properties.
            if (!libraryArgs.GetDestination().empty()) {
              artifactsSpecified = true;
            }
            if (namelinkMode != cmInstallTargetGenerator::NamelinkModeOnly) {
              libraryGenerator = CreateInstallTargetGenerator(
                target, libraryArgs, false, helper.Makefile->GetBacktrace(),
                helper.GetLibraryDestination(&libraryArgs));
              libraryGenerator->SetNamelinkMode(
                cmInstallTargetGenerator::NamelinkModeSkip);
            }
            if (namelinkMode != cmInstallTargetGenerator::NamelinkModeSkip) {
              namelinkGenerator = CreateInstallTargetGenerator(
                target, libraryArgs, false, helper.Makefile->GetBacktrace(),
                helper.GetLibraryDestination(&libraryArgs), false, true);
              namelinkGenerator->SetNamelinkMode(
                cmInstallTargetGenerator::NamelinkModeOnly);
            }
            namelinkOnly =
              (namelinkMode == cmInstallTargetGenerator::NamelinkModeOnly);
          }
        }
      } break;
      case cmStateEnums::STATIC_LIBRARY: {
        // If it is marked with FRAMEWORK property use the FRAMEWORK set of
        // INSTALL properties. Otherwise, use the LIBRARY properties.
        if (target.IsFrameworkOnApple()) {
          // When in namelink only mode skip frameworks.
          if (namelinkMode == cmInstallTargetGenerator::NamelinkModeOnly) {
            continue;
          }

          // Use the FRAMEWORK properties.
          if (!frameworkArgs.GetDestination().empty()) {
            frameworkGenerator = CreateInstallTargetGenerator(
              target, frameworkArgs, false, helper.Makefile->GetBacktrace());
          } else {
            status.SetError(
              cmStrCat("TARGETS given no FRAMEWORK DESTINATION for static "
                       "library FRAMEWORK target \"",
                       target.GetName(), "\"."));
            return false;
          }
        } else {
          // Static libraries use ARCHIVE properties.
          if (!archiveArgs.GetDestination().empty()) {
            artifactsSpecified = true;
          }
          archiveGenerator = CreateInstallTargetGenerator(
            target, archiveArgs, false, helper.Makefile->GetBacktrace(),
            helper.GetArchiveDestination(&archiveArgs));
        }
      } break;
      case cmStateEnums::MODULE_LIBRARY: {
        // Modules use LIBRARY properties.
        if (!libraryArgs.GetDestination().empty()) {
          libraryGenerator = CreateInstallTargetGenerator(
            target, libraryArgs, false, helper.Makefile->GetBacktrace());
          libraryGenerator->SetNamelinkMode(namelinkMode);
          namelinkOnly =
            (namelinkMode == cmInstallTargetGenerator::NamelinkModeOnly);
        } else {
          status.SetError(
            cmStrCat("TARGETS given no LIBRARY DESTINATION for module "
                     "target \"",
                     target.GetName(), "\"."));
          return false;
        }
      } break;
      case cmStateEnums::OBJECT_LIBRARY: {
        // Objects use OBJECT properties.
        if (!objectArgs.GetDestination().empty()) {
          // Verify that we know where the objects are to install them.
          std::string reason;
          if (!helper.Makefile->GetGlobalGenerator()
                 ->HasKnownObjectFileLocation(&reason)) {
            status.SetError(
              cmStrCat("TARGETS given OBJECT library \"", target.GetName(),
                       "\" whose objects may not be installed", reason, "."));
            return false;
          }

          objectGenerator = CreateInstallTargetGenerator(
            target, objectArgs, false, helper.Makefile->GetBacktrace());
        } else {
          // Installing an OBJECT library without a destination transforms
          // it to an INTERFACE library.  It installs no files but can be
          // exported.
        }
      } break;
      case cmStateEnums::EXECUTABLE: {
        if (target.IsAppBundleOnApple()) {
          // Application bundles use the BUNDLE properties.
          if (!bundleArgs.GetDestination().empty()) {
            bundleGenerator = CreateInstallTargetGenerator(
              target, bundleArgs, false, helper.Makefile->GetBacktrace());
          } else if (!runtimeArgs.GetDestination().empty()) {
            bool failure = false;
            if (helper.CheckCMP0006(failure)) {
              // For CMake 2.4 compatibility fallback to the RUNTIME
              // properties.
              bundleGenerator = CreateInstallTargetGenerator(
                target, runtimeArgs, false, helper.Makefile->GetBacktrace());
            } else if (failure) {
              return false;
            }
          }
          if (!bundleGenerator) {
            status.SetError(cmStrCat("TARGETS given no BUNDLE DESTINATION for "
                                     "MACOSX_BUNDLE executable target \"",
                                     target.GetName(), "\"."));
            return false;
          }
        } else {
          // Executables use the RUNTIME properties.
          if (!runtimeArgs.GetDestination().empty()) {
            artifactsSpecified = true;
          }
          runtimeGenerator = CreateInstallTargetGenerator(
            target, runtimeArgs, false, helper.Makefile->GetBacktrace(),
            helper.GetRuntimeDestination(&runtimeArgs));
        }

        // On DLL platforms an executable may also have an import
        // library.  Install it to the archive destination if it
        // exists.
        if ((target.IsDLLPlatform() || target.IsAIX()) &&
            !archiveArgs.GetDestination().empty() &&
            target.IsExecutableWithExports()) {
          // The import library uses the ARCHIVE properties.
          artifactsSpecified = true;
          archiveGenerator = CreateInstallTargetGenerator(
            target, archiveArgs, true, helper.Makefile->GetBacktrace(), true);
        }
      } break;
      case cmStateEnums::INTERFACE_LIBRARY:
        // Nothing to do. An INTERFACE_LIBRARY can be installed, but the
        // only effect of that is to make it exportable. It installs no
        // other files itself.
        break;
      default:
        // This should never happen due to the above type check.
        // Ignore the case.
        break;
    }

    // These well-known sets of files are installed *automatically* for
    // FRAMEWORK SHARED library targets on the Mac as part of installing the
    // FRAMEWORK.  For other target types or on other platforms, they are not
    // installed automatically and so we need to create install files
    // generators for them.
    bool createInstallGeneratorsForTargetFileSets = true;

    if (target.IsFrameworkOnApple()) {
      createInstallGeneratorsForTargetFileSets = false;
    }

    if (createInstallGeneratorsForTargetFileSets && !namelinkOnly) {
      const char* files = target.GetProperty("PRIVATE_HEADER");
      if ((files) && (*files)) {
        std::vector<std::string> relFiles = cmExpandedList(files);
        std::vector<std::string> absFiles;
        if (!helper.MakeFilesFullPath("PRIVATE_HEADER", relFiles, absFiles)) {
          return false;
        }

        // Create the files install generator.
        if (!artifactsSpecified ||
            !privateHeaderArgs.GetDestination().empty()) {
          privateHeaderGenerator = CreateInstallFilesGenerator(
            helper.Makefile, absFiles, privateHeaderArgs, false,
            helper.GetIncludeDestination(&privateHeaderArgs));
        } else {
          std::ostringstream e;
          e << "INSTALL TARGETS - target " << target.GetName() << " has "
            << "PRIVATE_HEADER files but no PRIVATE_HEADER DESTINATION.";
          cmSystemTools::Message(e.str(), "Warning");
        }
      }

      files = target.GetProperty("PUBLIC_HEADER");
      if ((files) && (*files)) {
        std::vector<std::string> relFiles = cmExpandedList(files);
        std::vector<std::string> absFiles;
        if (!helper.MakeFilesFullPath("PUBLIC_HEADER", relFiles, absFiles)) {
          return false;
        }

        // Create the files install generator.
        if (!artifactsSpecified ||
            !publicHeaderArgs.GetDestination().empty()) {
          publicHeaderGenerator = CreateInstallFilesGenerator(
            helper.Makefile, absFiles, publicHeaderArgs, false,
            helper.GetIncludeDestination(&publicHeaderArgs));
        } else {
          std::ostringstream e;
          e << "INSTALL TARGETS - target " << target.GetName() << " has "
            << "PUBLIC_HEADER files but no PUBLIC_HEADER DESTINATION.";
          cmSystemTools::Message(e.str(), "Warning");
        }
      }

      files = target.GetProperty("RESOURCE");
      if ((files) && (*files)) {
        std::vector<std::string> relFiles = cmExpandedList(files);
        std::vector<std::string> absFiles;
        if (!helper.MakeFilesFullPath("RESOURCE", relFiles, absFiles)) {
          return false;
        }

        // Create the files install generator.
        if (!resourceArgs.GetDestination().empty()) {
          resourceGenerator = CreateInstallFilesGenerator(
            helper.Makefile, absFiles, resourceArgs, false);
        } else {
          cmSystemTools::Message(
            cmStrCat("INSTALL TARGETS - target ", target.GetName(),
                     " has RESOURCE files but no RESOURCE DESTINATION."),
            "Warning");
        }
      }
    }

    // Add this install rule to an export if one was specified and
    // this is not a namelink-only rule.
    if (!exports.empty() && !namelinkOnly) {
      auto te = cm::make_unique<cmTargetExport>();
      te->TargetName = target.GetName();
      te->ArchiveGenerator = archiveGenerator.get();
      te->BundleGenerator = bundleGenerator.get();
      te->FrameworkGenerator = frameworkGenerator.get();
      te->HeaderGenerator = publicHeaderGenerator.get();
      te->LibraryGenerator = libraryGenerator.get();
      te->RuntimeGenerator = runtimeGenerator.get();
      te->ObjectsGenerator = objectGenerator.get();
      te->InterfaceIncludeDirectories =
        cmJoin(includesArgs.GetIncludeDirs(), ";");

      helper.Makefile->GetGlobalGenerator()
        ->GetExportSets()[exports]
        .AddTargetExport(std::move(te));
    }

    // Keep track of whether we're installing anything in each category
    installsArchive = installsArchive || archiveGenerator;
    installsLibrary = installsLibrary || libraryGenerator;
    installsNamelink = installsNamelink || namelinkGenerator;
    installsRuntime = installsRuntime || runtimeGenerator;
    installsObject = installsObject || objectGenerator;
    installsFramework = installsFramework || frameworkGenerator;
    installsBundle = installsBundle || bundleGenerator;
    installsPrivateHeader = installsPrivateHeader || privateHeaderGenerator;
    installsPublicHeader = installsPublicHeader || publicHeaderGenerator;
    installsResource = installsResource || resourceGenerator;

    helper.Makefile->AddInstallGenerator(std::move(archiveGenerator));
    helper.Makefile->AddInstallGenerator(std::move(libraryGenerator));
    helper.Makefile->AddInstallGenerator(std::move(namelinkGenerator));
    helper.Makefile->AddInstallGenerator(std::move(runtimeGenerator));
    helper.Makefile->AddInstallGenerator(std::move(objectGenerator));
    helper.Makefile->AddInstallGenerator(std::move(frameworkGenerator));
    helper.Makefile->AddInstallGenerator(std::move(bundleGenerator));
    helper.Makefile->AddInstallGenerator(std::move(privateHeaderGenerator));
    helper.Makefile->AddInstallGenerator(std::move(publicHeaderGenerator));
    helper.Makefile->AddInstallGenerator(std::move(resourceGenerator));
  }

  // Tell the global generator about any installation component names
  // specified
  if (installsArchive) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      archiveArgs.GetComponent());
  }
  if (installsLibrary) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      libraryArgs.GetComponent());
  }
  if (installsNamelink) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      libraryArgs.GetNamelinkComponent());
  }
  if (installsRuntime) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      runtimeArgs.GetComponent());
  }
  if (installsObject) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      objectArgs.GetComponent());
  }
  if (installsFramework) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      frameworkArgs.GetComponent());
  }
  if (installsBundle) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      bundleArgs.GetComponent());
  }
  if (installsPrivateHeader) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      privateHeaderArgs.GetComponent());
  }
  if (installsPublicHeader) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      publicHeaderArgs.GetComponent());
  }
  if (installsResource) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      resourceArgs.GetComponent());
  }

  return true;
}

bool HandleFilesMode(std::vector<std::string> const& args,
                     cmExecutionStatus& status)
{
  Helper helper(status);

  // This is the FILES mode.
  bool programs = (args[0] == "PROGRAMS");
  cmInstallCommandArguments ica(helper.DefaultComponentName);
  std::vector<std::string> files;
  ica.Bind(programs ? "PROGRAMS"_s : "FILES"_s, files);
  std::vector<std::string> unknownArgs;
  ica.Parse(args, &unknownArgs);

  if (!unknownArgs.empty()) {
    // Unknown argument.
    status.SetError(
      cmStrCat(args[0], " given unknown argument \"", unknownArgs[0], "\"."));
    return false;
  }

  std::string type = ica.GetType();
  if (!type.empty() && allowedTypes.count(type) == 0) {
    status.SetError(
      cmStrCat(args[0], " given non-type \"", type, "\" with TYPE argument."));
    return false;
  }

  const std::vector<std::string>& filesVector = files;

  // Check if there is something to do.
  if (filesVector.empty()) {
    return true;
  }

  if (!ica.GetRename().empty() && filesVector.size() > 1) {
    // The rename option works only with one file.
    status.SetError(
      cmStrCat(args[0], " given RENAME option with more than one file."));
    return false;
  }

  std::vector<std::string> absFiles;
  if (!helper.MakeFilesFullPath(args[0].c_str(), filesVector, absFiles)) {
    return false;
  }

  cmPolicies::PolicyStatus policyStatus =
    helper.Makefile->GetPolicyStatus(cmPolicies::CMP0062);

  cmGlobalGenerator* gg = helper.Makefile->GetGlobalGenerator();
  for (std::string const& file : filesVector) {
    if (gg->IsExportedTargetsFile(file)) {
      const char* modal = nullptr;
      std::ostringstream e;
      MessageType messageType = MessageType::AUTHOR_WARNING;

      switch (policyStatus) {
        case cmPolicies::WARN:
          e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0062) << "\n";
          modal = "should";
        case cmPolicies::OLD:
          break;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
        case cmPolicies::NEW:
          modal = "may";
          messageType = MessageType::FATAL_ERROR;
      }
      if (modal) {
        e << "The file\n  " << file
          << "\nwas generated by the export() "
             "command.  It "
          << modal
          << " not be installed with the "
             "install() command.  Use the install(EXPORT) mechanism "
             "instead.  See the cmake-packages(7) manual for more.\n";
        helper.Makefile->IssueMessage(messageType, e.str());
        if (messageType == MessageType::FATAL_ERROR) {
          return false;
        }
      }
    }
  }

  if (!ica.Finalize()) {
    return false;
  }

  if (!type.empty() && !ica.GetDestination().empty()) {
    status.SetError(cmStrCat(args[0],
                             " given both TYPE and DESTINATION arguments. "
                             "You may only specify one."));
    return false;
  }

  std::string destination = helper.GetDestinationForType(&ica, type);
  if (destination.empty()) {
    // A destination is required.
    status.SetError(cmStrCat(args[0], " given no DESTINATION!"));
    return false;
  }

  // Create the files install generator.
  helper.Makefile->AddInstallGenerator(CreateInstallFilesGenerator(
    helper.Makefile, absFiles, ica, programs, destination));

  // Tell the global generator about any installation component names
  // specified.
  helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
    ica.GetComponent());

  return true;
}

bool HandleDirectoryMode(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  Helper helper(status);

  enum Doing
  {
    DoingNone,
    DoingDirs,
    DoingDestination,
    DoingPattern,
    DoingRegex,
    DoingPermsFile,
    DoingPermsDir,
    DoingPermsMatch,
    DoingConfigurations,
    DoingComponent,
    DoingType
  };
  Doing doing = DoingDirs;
  bool in_match_mode = false;
  bool optional = false;
  bool exclude_from_all = false;
  bool message_never = false;
  std::vector<std::string> dirs;
  const std::string* destination = nullptr;
  std::string permissions_file;
  std::string permissions_dir;
  std::vector<std::string> configurations;
  std::string component = helper.DefaultComponentName;
  std::string literal_args;
  std::string type;
  for (unsigned int i = 1; i < args.size(); ++i) {
    if (args[i] == "DESTINATION") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }

      // Switch to setting the destination property.
      doing = DoingDestination;
    } else if (args[i] == "TYPE") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }

      // Switch to setting the type.
      doing = DoingType;
    } else if (args[i] == "OPTIONAL") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }

      // Mark the rule as optional.
      optional = true;
      doing = DoingNone;
    } else if (args[i] == "MESSAGE_NEVER") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }

      // Mark the rule as quiet.
      message_never = true;
      doing = DoingNone;
    } else if (args[i] == "PATTERN") {
      // Switch to a new pattern match rule.
      doing = DoingPattern;
      in_match_mode = true;
    } else if (args[i] == "REGEX") {
      // Switch to a new regex match rule.
      doing = DoingRegex;
      in_match_mode = true;
    } else if (args[i] == "EXCLUDE") {
      // Add this property to the current match rule.
      if (!in_match_mode || doing == DoingPattern || doing == DoingRegex) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" before a PATTERN or REGEX is given."));
        return false;
      }
      literal_args += " EXCLUDE";
      doing = DoingNone;
    } else if (args[i] == "PERMISSIONS") {
      if (!in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" before a PATTERN or REGEX is given."));
        return false;
      }

      // Switch to setting the current match permissions property.
      literal_args += " PERMISSIONS";
      doing = DoingPermsMatch;
    } else if (args[i] == "FILE_PERMISSIONS") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }

      // Switch to setting the file permissions property.
      doing = DoingPermsFile;
    } else if (args[i] == "DIRECTORY_PERMISSIONS") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }

      // Switch to setting the directory permissions property.
      doing = DoingPermsDir;
    } else if (args[i] == "USE_SOURCE_PERMISSIONS") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }

      // Add this option literally.
      literal_args += " USE_SOURCE_PERMISSIONS";
      doing = DoingNone;
    } else if (args[i] == "FILES_MATCHING") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }

      // Add this option literally.
      literal_args += " FILES_MATCHING";
      doing = DoingNone;
    } else if (args[i] == "CONFIGURATIONS") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }

      // Switch to setting the configurations property.
      doing = DoingConfigurations;
    } else if (args[i] == "COMPONENT") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }

      // Switch to setting the component property.
      doing = DoingComponent;
    } else if (args[i] == "EXCLUDE_FROM_ALL") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }
      exclude_from_all = true;
      doing = DoingNone;
    } else if (doing == DoingDirs) {
      // Convert this directory to a full path.
      std::string dir = args[i];
      std::string::size_type gpos = cmGeneratorExpression::Find(dir);
      if (gpos != 0 && !cmSystemTools::FileIsFullPath(dir)) {
        dir =
          cmStrCat(helper.Makefile->GetCurrentSourceDirectory(), '/', args[i]);
      }

      // Make sure the name is a directory.
      if (cmSystemTools::FileExists(dir) &&
          !cmSystemTools::FileIsDirectory(dir)) {
        status.SetError(cmStrCat(args[0], " given non-directory \"", args[i],
                                 "\" to install."));
        return false;
      }

      // Store the directory for installation.
      dirs.push_back(std::move(dir));
    } else if (doing == DoingConfigurations) {
      configurations.push_back(args[i]);
    } else if (doing == DoingDestination) {
      destination = &args[i];
      doing = DoingNone;
    } else if (doing == DoingType) {
      if (allowedTypes.count(args[i]) == 0) {
        status.SetError(cmStrCat(args[0], " given non-type \"", args[i],
                                 "\" with TYPE argument."));
        return false;
      }

      type = args[i];
      doing = DoingNone;
    } else if (doing == DoingPattern) {
      // Convert the pattern to a regular expression.  Require a
      // leading slash and trailing end-of-string in the matched
      // string to make sure the pattern matches only whole file
      // names.
      literal_args += " REGEX \"/";
      std::string regex = cmsys::Glob::PatternToRegex(args[i], false);
      cmSystemTools::ReplaceString(regex, "\\", "\\\\");
      literal_args += regex;
      literal_args += "$\"";
      doing = DoingNone;
    } else if (doing == DoingRegex) {
      literal_args += " REGEX \"";
// Match rules are case-insensitive on some platforms.
#if defined(_WIN32) || defined(__APPLE__) || defined(__CYGWIN__)
      std::string regex = cmSystemTools::LowerCase(args[i]);
#else
      std::string regex = args[i];
#endif
      cmSystemTools::ReplaceString(regex, "\\", "\\\\");
      literal_args += regex;
      literal_args += "\"";
      doing = DoingNone;
    } else if (doing == DoingComponent) {
      component = args[i];
      doing = DoingNone;
    } else if (doing == DoingPermsFile) {
      // Check the requested permission.
      if (!cmInstallCommandArguments::CheckPermissions(args[i],
                                                       permissions_file)) {
        status.SetError(cmStrCat(args[0], " given invalid file permission \"",
                                 args[i], "\"."));
        return false;
      }
    } else if (doing == DoingPermsDir) {
      // Check the requested permission.
      if (!cmInstallCommandArguments::CheckPermissions(args[i],
                                                       permissions_dir)) {
        status.SetError(cmStrCat(
          args[0], " given invalid directory permission \"", args[i], "\"."));
        return false;
      }
    } else if (doing == DoingPermsMatch) {
      // Check the requested permission.
      if (!cmInstallCommandArguments::CheckPermissions(args[i],
                                                       literal_args)) {
        status.SetError(
          cmStrCat(args[0], " given invalid permission \"", args[i], "\"."));
        return false;
      }
    } else {
      // Unknown argument.
      status.SetError(
        cmStrCat(args[0], " given unknown argument \"", args[i], "\"."));
      return false;
    }
  }

  // Support installing an empty directory.
  if (dirs.empty() && destination) {
    dirs.emplace_back();
  }

  // Check if there is something to do.
  if (dirs.empty()) {
    return true;
  }
  std::string destinationStr;
  if (!destination) {
    if (type.empty()) {
      // A destination is required.
      status.SetError(cmStrCat(args[0], " given no DESTINATION!"));
      return false;
    }
    destinationStr = helper.GetDestinationForType(nullptr, type);
    destination = &destinationStr;
  } else if (!type.empty()) {
    status.SetError(cmStrCat(args[0],
                             " given both TYPE and DESTINATION "
                             "arguments. You may only specify one."));
    return false;
  }

  cmInstallGenerator::MessageLevel message =
    cmInstallGenerator::SelectMessageLevel(helper.Makefile, message_never);

  // Create the directory install generator.
  helper.Makefile->AddInstallGenerator(
    cm::make_unique<cmInstallDirectoryGenerator>(
      dirs, *destination, permissions_file, permissions_dir, configurations,
      component, message, exclude_from_all, literal_args, optional));

  // Tell the global generator about any installation component names
  // specified.
  helper.Makefile->GetGlobalGenerator()->AddInstallComponent(component);

  return true;
}

bool HandleExportAndroidMKMode(std::vector<std::string> const& args,
                               cmExecutionStatus& status)
{
#ifndef CMAKE_BOOTSTRAP
  Helper helper(status);

  // This is the EXPORT mode.
  cmInstallCommandArguments ica(helper.DefaultComponentName);

  std::string exp;
  std::string name_space;
  bool exportOld = false;
  std::string filename;

  ica.Bind("EXPORT_ANDROID_MK"_s, exp);
  ica.Bind("NAMESPACE"_s, name_space);
  ica.Bind("EXPORT_LINK_INTERFACE_LIBRARIES"_s, exportOld);
  ica.Bind("FILE"_s, filename);

  std::vector<std::string> unknownArgs;
  ica.Parse(args, &unknownArgs);

  if (!unknownArgs.empty()) {
    // Unknown argument.
    status.SetError(
      cmStrCat(args[0], " given unknown argument \"", unknownArgs[0], "\"."));
    return false;
  }

  if (!ica.Finalize()) {
    return false;
  }

  // Make sure there is a destination.
  if (ica.GetDestination().empty()) {
    // A destination is required.
    status.SetError(cmStrCat(args[0], " given no DESTINATION!"));
    return false;
  }

  // Check the file name.
  std::string fname = filename;
  if (fname.find_first_of(":/\\") != std::string::npos) {
    status.SetError(cmStrCat(args[0], " given invalid export file name \"",
                             fname,
                             "\".  The FILE argument may not contain a path.  "
                             "Specify the path in the DESTINATION argument."));
    return false;
  }

  // Check the file extension.
  if (!fname.empty() &&
      cmSystemTools::GetFilenameLastExtension(fname) != ".mk") {
    status.SetError(cmStrCat(
      args[0], " given invalid export file name \"", fname,
      R"(".  The FILE argument must specify a name ending in ".mk".)"));
    return false;
  }
  if (fname.find_first_of(":/\\") != std::string::npos) {
    status.SetError(
      cmStrCat(args[0], " given export name \"", exp,
               "\".  "
               "This name cannot be safely converted to a file name.  "
               "Specify a different export name or use the FILE option to set "
               "a file name explicitly."));
    return false;
  }
  // Use the default name
  if (fname.empty()) {
    fname = "Android.mk";
  }

  cmExportSet& exportSet =
    helper.Makefile->GetGlobalGenerator()->GetExportSets()[exp];

  cmInstallGenerator::MessageLevel message =
    cmInstallGenerator::SelectMessageLevel(helper.Makefile);

  // Create the export install generator.
  helper.Makefile->AddInstallGenerator(
    cm::make_unique<cmInstallExportGenerator>(
      &exportSet, ica.GetDestination(), ica.GetPermissions(),
      ica.GetConfigurations(), ica.GetComponent(), message,
      ica.GetExcludeFromAll(), fname, name_space, exportOld, true));

  return true;
#else
  static_cast<void>(args);
  status.SetError("EXPORT_ANDROID_MK not supported in bootstrap cmake");
  return false;
#endif
}

bool HandleExportMode(std::vector<std::string> const& args,
                      cmExecutionStatus& status)
{
  Helper helper(status);

  // This is the EXPORT mode.
  cmInstallCommandArguments ica(helper.DefaultComponentName);

  std::string exp;
  std::string name_space;
  bool exportOld = false;
  std::string filename;

  ica.Bind("EXPORT"_s, exp);
  ica.Bind("NAMESPACE"_s, name_space);
  ica.Bind("EXPORT_LINK_INTERFACE_LIBRARIES"_s, exportOld);
  ica.Bind("FILE"_s, filename);

  std::vector<std::string> unknownArgs;
  ica.Parse(args, &unknownArgs);

  if (!unknownArgs.empty()) {
    // Unknown argument.
    status.SetError(
      cmStrCat(args[0], " given unknown argument \"", unknownArgs[0], "\"."));
    return false;
  }

  if (!ica.Finalize()) {
    return false;
  }

  // Make sure there is a destination.
  if (ica.GetDestination().empty()) {
    // A destination is required.
    status.SetError(cmStrCat(args[0], " given no DESTINATION!"));
    return false;
  }

  // Check the file name.
  std::string fname = filename;
  if (fname.find_first_of(":/\\") != std::string::npos) {
    status.SetError(cmStrCat(args[0], " given invalid export file name \"",
                             fname,
                             "\".  "
                             "The FILE argument may not contain a path.  "
                             "Specify the path in the DESTINATION argument."));
    return false;
  }

  // Check the file extension.
  if (!fname.empty() &&
      cmSystemTools::GetFilenameLastExtension(fname) != ".cmake") {
    status.SetError(
      cmStrCat(args[0], " given invalid export file name \"", fname,
               "\".  "
               "The FILE argument must specify a name ending in \".cmake\"."));
    return false;
  }

  // Construct the file name.
  if (fname.empty()) {
    fname = cmStrCat(exp, ".cmake");

    if (fname.find_first_of(":/\\") != std::string::npos) {
      status.SetError(cmStrCat(
        args[0], " given export name \"", exp,
        "\".  "
        "This name cannot be safely converted to a file name.  "
        "Specify a different export name or use the FILE option to set "
        "a file name explicitly."));
      return false;
    }
  }

  cmExportSet& exportSet =
    helper.Makefile->GetGlobalGenerator()->GetExportSets()[exp];
  if (exportOld) {
    for (auto const& te : exportSet.GetTargetExports()) {
      cmTarget* tgt =
        helper.Makefile->GetGlobalGenerator()->FindTarget(te->TargetName);
      const bool newCMP0022Behavior =
        (tgt && tgt->GetPolicyStatusCMP0022() != cmPolicies::WARN &&
         tgt->GetPolicyStatusCMP0022() != cmPolicies::OLD);

      if (!newCMP0022Behavior) {
        status.SetError(cmStrCat(
          "INSTALL(EXPORT) given keyword \""
          "EXPORT_LINK_INTERFACE_LIBRARIES\", but target \"",
          te->TargetName, "\" does not have policy CMP0022 set to NEW."));
        return false;
      }
    }
  }

  cmInstallGenerator::MessageLevel message =
    cmInstallGenerator::SelectMessageLevel(helper.Makefile);

  // Create the export install generator.
  helper.Makefile->AddInstallGenerator(
    cm::make_unique<cmInstallExportGenerator>(
      &exportSet, ica.GetDestination(), ica.GetPermissions(),
      ica.GetConfigurations(), ica.GetComponent(), message,
      ica.GetExcludeFromAll(), fname, name_space, exportOld, false));

  return true;
}

bool Helper::MakeFilesFullPath(const char* modeName,
                               const std::vector<std::string>& relFiles,
                               std::vector<std::string>& absFiles)
{
  for (std::string const& relFile : relFiles) {
    std::string file = relFile;
    std::string::size_type gpos = cmGeneratorExpression::Find(file);
    if (gpos != 0 && !cmSystemTools::FileIsFullPath(file)) {
      file =
        cmStrCat(this->Makefile->GetCurrentSourceDirectory(), '/', relFile);
    }

    // Make sure the file is not a directory.
    if (gpos == std::string::npos && cmSystemTools::FileIsDirectory(file)) {
      this->SetError(
        cmStrCat(modeName, " given directory \"", relFile, "\" to install."));
      return false;
    }
    // Store the file for installation.
    absFiles.push_back(std::move(file));
  }
  return true;
}

bool Helper::CheckCMP0006(bool& failure)
{
  switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0006)) {
    case cmPolicies::WARN:
      this->Makefile->IssueMessage(
        MessageType::AUTHOR_WARNING,
        cmPolicies::GetPolicyWarning(cmPolicies::CMP0006));
      CM_FALLTHROUGH;
    case cmPolicies::OLD:
      // OLD behavior is to allow compatibility
      return true;
    case cmPolicies::NEW:
      // NEW behavior is to disallow compatibility
      break;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
      failure = true;
      this->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0006));
      break;
  }
  return false;
}

std::string Helper::GetDestination(const cmInstallCommandArguments* args,
                                   const std::string& varName,
                                   const std::string& guess)
{
  if (args && !args->GetDestination().empty()) {
    return args->GetDestination();
  }
  std::string val = this->Makefile->GetSafeDefinition(varName);
  if (!val.empty()) {
    return val;
  }
  return guess;
}

std::string Helper::GetRuntimeDestination(
  const cmInstallCommandArguments* args)
{
  return this->GetDestination(args, "CMAKE_INSTALL_BINDIR", "bin");
}

std::string Helper::GetSbinDestination(const cmInstallCommandArguments* args)
{
  return this->GetDestination(args, "CMAKE_INSTALL_SBINDIR", "sbin");
}

std::string Helper::GetArchiveDestination(
  const cmInstallCommandArguments* args)
{
  return this->GetDestination(args, "CMAKE_INSTALL_LIBDIR", "lib");
}

std::string Helper::GetLibraryDestination(
  const cmInstallCommandArguments* args)
{
  return this->GetDestination(args, "CMAKE_INSTALL_LIBDIR", "lib");
}

std::string Helper::GetIncludeDestination(
  const cmInstallCommandArguments* args)
{
  return this->GetDestination(args, "CMAKE_INSTALL_INCLUDEDIR", "include");
}

std::string Helper::GetSysconfDestination(
  const cmInstallCommandArguments* args)
{
  return this->GetDestination(args, "CMAKE_INSTALL_SYSCONFDIR", "etc");
}

std::string Helper::GetSharedStateDestination(
  const cmInstallCommandArguments* args)
{
  return this->GetDestination(args, "CMAKE_INSTALL_SHAREDSTATEDIR", "com");
}

std::string Helper::GetLocalStateDestination(
  const cmInstallCommandArguments* args)
{
  return this->GetDestination(args, "CMAKE_INSTALL_LOCALSTATEDIR", "var");
}

std::string Helper::GetRunStateDestination(
  const cmInstallCommandArguments* args)
{
  return this->GetDestination(args, "CMAKE_INSTALL_RUNSTATEDIR",
                              this->GetLocalStateDestination(nullptr) +
                                "/run");
}

std::string Helper::GetDataRootDestination(
  const cmInstallCommandArguments* args)
{
  return this->GetDestination(args, "CMAKE_INSTALL_DATAROOTDIR", "share");
}

std::string Helper::GetDataDestination(const cmInstallCommandArguments* args)
{
  return this->GetDestination(args, "CMAKE_INSTALL_DATADIR",
                              this->GetDataRootDestination(nullptr));
}

std::string Helper::GetInfoDestination(const cmInstallCommandArguments* args)
{
  return this->GetDestination(args, "CMAKE_INSTALL_INFODIR",
                              this->GetDataRootDestination(nullptr) + "/info");
}

std::string Helper::GetLocaleDestination(const cmInstallCommandArguments* args)
{
  return this->GetDestination(args, "CMAKE_INSTALL_LOCALEDIR",
                              this->GetDataRootDestination(nullptr) +
                                "/locale");
}

std::string Helper::GetManDestination(const cmInstallCommandArguments* args)
{
  return this->GetDestination(args, "CMAKE_INSTALL_MANDIR",
                              this->GetDataRootDestination(nullptr) + "/man");
}

std::string Helper::GetDocDestination(const cmInstallCommandArguments* args)
{
  return this->GetDestination(args, "CMAKE_INSTALL_DOCDIR",
                              this->GetDataRootDestination(nullptr) + "/doc");
}

std::string Helper::GetDestinationForType(
  const cmInstallCommandArguments* args, const std::string& type)
{
  if (args && !args->GetDestination().empty()) {
    return args->GetDestination();
  }
  if (type == "BIN") {
    return this->GetRuntimeDestination(nullptr);
  }
  if (type == "SBIN") {
    return this->GetSbinDestination(nullptr);
  }
  if (type == "SYSCONF") {
    return this->GetSysconfDestination(nullptr);
  }
  if (type == "SHAREDSTATE") {
    return this->GetSharedStateDestination(nullptr);
  }
  if (type == "LOCALSTATE") {
    return this->GetLocalStateDestination(nullptr);
  }
  if (type == "RUNSTATE") {
    return this->GetRunStateDestination(nullptr);
  }
  if (type == "LIB") {
    return this->GetLibraryDestination(nullptr);
  }
  if (type == "INCLUDE") {
    return this->GetIncludeDestination(nullptr);
  }
  if (type == "DATA") {
    return this->GetDataDestination(nullptr);
  }
  if (type == "INFO") {
    return this->GetInfoDestination(nullptr);
  }
  if (type == "LOCALE") {
    return this->GetLocaleDestination(nullptr);
  }
  if (type == "MAN") {
    return this->GetManDestination(nullptr);
  }
  if (type == "DOC") {
    return this->GetDocDestination(nullptr);
  }
  return "";
}

} // namespace

bool cmInstallCommand(std::vector<std::string> const& args,
                      cmExecutionStatus& status)
{
  // Allow calling with no arguments so that arguments may be built up
  // using a variable that may be left empty.
  if (args.empty()) {
    return true;
  }

  // Enable the install target.
  status.GetMakefile().GetGlobalGenerator()->EnableInstallTarget();

  static cmSubcommandTable const subcommand{
    { "SCRIPT"_s, HandleScriptMode },
    { "CODE"_s, HandleScriptMode },
    { "TARGETS"_s, HandleTargetsMode },
    { "FILES"_s, HandleFilesMode },
    { "PROGRAMS"_s, HandleFilesMode },
    { "DIRECTORY"_s, HandleDirectoryMode },
    { "EXPORT"_s, HandleExportMode },
    { "EXPORT_ANDROID_MK"_s, HandleExportAndroidMKMode },
  };

  return subcommand(args[0], args, status);
}
