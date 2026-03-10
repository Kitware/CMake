/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
/* clang-format off */
#include "cmGeneratorTarget.h"
/* clang-format on */

#include <algorithm>
#include <array>
#include <initializer_list>
#include <map>
#include <ostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/optional>
#include <cm/string_view>

#include "cmDiagnostics.h"
#include "cmFileSetMetadata.h"
#include "cmGenExContext.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorFileSet.h"
#include "cmGlobalGenerator.h"
#include "cmList.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocation.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetLinkLibraryType.h"
#include "cmValue.h"

bool cmGeneratorTarget::AddHeaderSetVerification()
{
  for (bool const isInterface : { false, true }) {
    if (!this->GetPropertyAsBool(isInterface ? "VERIFY_INTERFACE_HEADER_SETS"
                                             : "VERIFY_PRIVATE_HEADER_SETS")) {
      continue;
    }

    if (this->GetType() != cmStateEnums::STATIC_LIBRARY &&
        this->GetType() != cmStateEnums::SHARED_LIBRARY &&
        (this->GetType() != cmStateEnums::MODULE_LIBRARY || isInterface) &&
        this->GetType() != cmStateEnums::UNKNOWN_LIBRARY &&
        this->GetType() != cmStateEnums::OBJECT_LIBRARY &&
        this->GetType() != cmStateEnums::INTERFACE_LIBRARY &&
        this->GetType() != cmStateEnums::EXECUTABLE) {
      continue;
    }

    char const* headerSetsProperty = isInterface
      ? "INTERFACE_HEADER_SETS_TO_VERIFY"
      : "PRIVATE_HEADER_SETS_TO_VERIFY";

    auto verifyValue = this->GetProperty(headerSetsProperty);
    bool const all = verifyValue.IsEmpty();
    std::set<std::string> verifySet;
    if (!all) {
      cmList verifyList{ verifyValue };
      verifySet.insert(verifyList.begin(), verifyList.end());
    }

    cmTarget* verifyTarget = nullptr;
    std::string const verifyTargetName =
      cmStrCat(this->GetName(),
               isInterface ? "_verify_interface_header_sets"
                           : "_verify_private_header_sets");

    char const* allVerifyTargetName = isInterface
      ? "all_verify_interface_header_sets"
      : "all_verify_private_header_sets";
    cmTarget* allVerifyTarget =
      this->GlobalGenerator->GetMakefiles().front()->FindTargetToUse(
        allVerifyTargetName, { cmStateEnums::TargetDomain::NATIVE });

    auto fileSetEntries = isInterface
      ? this->GetInterfaceFileSets(cm::FileSetMetadata::HEADERS)
      : this->GetFileSets(cm::FileSetMetadata::HEADERS);

    std::set<cmGeneratorFileSet const*> fileSets;
    for (auto const& fileSet : fileSetEntries) {
      if (all || verifySet.count(fileSet->GetName())) {
        fileSets.insert(fileSet);
        verifySet.erase(fileSet->GetName());
      }
    }

    if (isInterface) {
      cmPolicies::PolicyStatus const cmp0209 = this->GetPolicyStatusCMP0209();
      if (cmp0209 != cmPolicies::NEW &&
          this->GetType() == cmStateEnums::EXECUTABLE &&
          !this->GetPropertyAsBool("ENABLE_EXPORTS")) {
        if (cmp0209 == cmPolicies::WARN && !fileSets.empty()) {
          this->Makefile->IssueDiagnostic(
            cmDiagnostics::CMD_AUTHOR,
            cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0209),
                     "\n"
                     "Executable target \"",
                     this->GetName(),
                     "\" has interface header file sets, but it does not "
                     "enable exports. Those headers would be verified under "
                     "CMP0209 NEW behavior.\n"));
        }
        continue;
      }
    }

    if (!verifySet.empty()) {
      this->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("Property ", headerSetsProperty, " of target \"",
                 this->GetName(),
                 "\" contained the following header sets that are nonexistent "
                 "or not ",
                 isInterface ? "INTERFACE" : "PRIVATE", ":\n  ",
                 cmJoin(verifySet, "\n  ")));
      return false;
    }

    cm::optional<cm::optional<std::string>> defaultLanguage;

    // First, collect all verification stubs before creating the target,
    // so we know whether to create an OBJECT library or not.
    std::vector<std::string> stubSources;
    for (auto const* fileSet : fileSets) {
      auto const& dirCges = fileSet->CompileDirectoryEntries();
      auto const& fileCges = fileSet->CompileFileEntries();

      static auto const contextSensitive =
        [](std::unique_ptr<cmCompiledGeneratorExpression> const& cge) {
          return cge->GetHadContextSensitiveCondition();
        };
      bool dirCgesContextSensitive = false;
      bool fileCgesContextSensitive = false;

      std::vector<std::string> dirs;
      std::map<std::string, std::vector<std::string>> filesPerDir;
      bool first = true;
      for (auto const& config : this->Makefile->GetGeneratorConfigs(
             cmMakefile::GeneratorConfigQuery::IncludeEmptyConfig)) {
        cm::GenEx::Context context(this->LocalGenerator, config);
        if (first || dirCgesContextSensitive) {
          dirs = fileSet->EvaluateDirectoryEntries(dirCges, context, this);
          dirCgesContextSensitive =
            std::any_of(dirCges.begin(), dirCges.end(), contextSensitive);
        }
        if (first || fileCgesContextSensitive) {
          filesPerDir.clear();
          for (auto const& fileCge : fileCges) {
            fileSet->EvaluateFileEntry(dirs, filesPerDir, fileCge, context,
                                       this);
            if (fileCge->GetHadContextSensitiveCondition()) {
              fileCgesContextSensitive = true;
            }
          }
        }

        for (auto const& files : filesPerDir) {
          for (auto const& file : files.second) {
            cm::optional<std::string> filenameOpt =
              this->GenerateHeaderSetVerificationFile(
                *this->Makefile->GetOrCreateSource(file), files.first,
                verifyTargetName, defaultLanguage);
            if (!filenameOpt) {
              continue;
            }
            std::string filename = *filenameOpt;

            if (fileCgesContextSensitive) {
              filename = cmStrCat("$<$<CONFIG:", config, ">:", filename, '>');
            }
            stubSources.emplace_back(std::move(filename));
          }
        }

        if (!dirCgesContextSensitive && !fileCgesContextSensitive) {
          break;
        }
        first = false;
      }
    }

    if (stubSources.empty()) {
      // No headers to verify. Create a utility target so the target
      // name always exists (e.g. for build system dependencies) without
      // needing a placeholder source. This avoids warnings from tools
      // like Xcode's libtool about empty static libraries.
      verifyTarget =
        this->Makefile->AddNewUtilityTarget(verifyTargetName, true);
    } else {
      // Create an OBJECT library to compile the verification stubs.
      {
        cmMakefile::PolicyPushPop polScope(this->Makefile);
        this->Makefile->SetPolicy(cmPolicies::CMP0119, cmPolicies::NEW);
        verifyTarget = this->Makefile->AddLibrary(
          verifyTargetName, cmStateEnums::OBJECT_LIBRARY, {}, true);
      }

      if (isInterface) {
        // Link to the original target so that we pick up its
        // interface compile options just like a consumer would.
        // This also ensures any generated headers in the original
        // target will be created.
        verifyTarget->AddLinkLibrary(
          *this->Makefile, this->GetName(),
          cmTargetLinkLibraryType::GENERAL_LibraryType);
      } else {
        // For private file sets, we need to simulate compiling the
        // same way as the original target. That includes linking to
        // the same things so we pick up the same transitive
        // properties. For the <LANG>_... properties, we don't care if
        // we set them for languages this target won't eventually use.
        // Copy language-standard properties for all supported
        // languages. We don't care if we set properties for languages
        // this target won't eventually use.
        static std::array<std::string, 19> const propertiesToCopy{ {
          "COMPILE_DEFINITIONS",    "COMPILE_FEATURES",
          "COMPILE_FLAGS",          "COMPILE_OPTIONS",
          "DEFINE_SYMBOL",          "INCLUDE_DIRECTORIES",
          "LINK_LIBRARIES",         "C_STANDARD",
          "C_STANDARD_REQUIRED",    "C_EXTENSIONS",
          "CXX_STANDARD",           "CXX_STANDARD_REQUIRED",
          "CXX_EXTENSIONS",         "OBJC_STANDARD",
          "OBJC_STANDARD_REQUIRED", "OBJC_EXTENSIONS",
          "OBJCXX_STANDARD",        "OBJCXX_STANDARD_REQUIRED",
          "OBJCXX_EXTENSIONS",
        } };
        for (std::string const& prop : propertiesToCopy) {
          cmValue propValue = this->Target->GetProperty(prop);
          if (propValue.IsSet()) {
            verifyTarget->SetProperty(prop, propValue);
          }
        }
        // The original target might have generated headers. Since
        // we only link to the original target for compilation,
        // there's nothing to force such generation to happen yet.
        // Our verify target must depend on the original target to
        // ensure such generated files will be created.
        verifyTarget->AddUtility(this->GetName(), false, this->Makefile);
        verifyTarget->AddCodegenDependency(this->GetName());
      }

      verifyTarget->SetProperty("AUTOMOC", "OFF");
      verifyTarget->SetProperty("AUTORCC", "OFF");
      verifyTarget->SetProperty("AUTOUIC", "OFF");
      verifyTarget->SetProperty("DISABLE_PRECOMPILE_HEADERS", "ON");
      verifyTarget->SetProperty("UNITY_BUILD", "OFF");
      verifyTarget->SetProperty("CXX_SCAN_FOR_MODULES", "OFF");

      if (isInterface) {
        verifyTarget->FinalizeTargetConfiguration(
          this->Makefile->GetCompileDefinitionsEntries());
      } else {
        // Private verification only needs to add the directory scope
        // definitions here
        for (auto const& def :
             this->Makefile->GetCompileDefinitionsEntries()) {
          verifyTarget->InsertCompileDefinition(def);
        }
      }

      for (auto const& source : stubSources) {
        verifyTarget->AddSource(source);
      }
    }
    if (!allVerifyTarget) {
      allVerifyTarget =
        this->GlobalGenerator->GetMakefiles().front()->AddNewUtilityTarget(
          allVerifyTargetName, true);
    }
    allVerifyTarget->AddUtility(verifyTargetName, false);

    this->LocalGenerator->AddGeneratorTarget(
      cm::make_unique<cmGeneratorTarget>(verifyTarget, this->LocalGenerator));
  }
  return true;
}

cm::optional<std::string> cmGeneratorTarget::GenerateHeaderSetVerificationFile(
  cmSourceFile& source, std::string const& dir,
  std::string const& verifyTargetName,
  cm::optional<cm::optional<std::string>>& defaultLanguage) const
{
  if (source.GetPropertyAsBool("SKIP_LINTING")) {
    return cm::nullopt;
  }

  cm::optional<std::string> language =
    this->ResolveHeaderLanguage(source, defaultLanguage);
  if (!language) {
    return cm::nullopt;
  }

  std::string headerFilename = dir;
  if (!headerFilename.empty()) {
    headerFilename += '/';
  }
  headerFilename += source.GetLocation().GetName();

  return this->GenerateStubForLanguage(*language, headerFilename,
                                       verifyTargetName, source);
}

cm::optional<std::string> cmGeneratorTarget::ResolveHeaderLanguage(
  cmSourceFile& source,
  cm::optional<cm::optional<std::string>>& defaultLanguage) const
{
  static std::array<cm::string_view, 4> const supportedLangs{ {
    "C",
    "CXX",
    "OBJC",
    "OBJCXX",
  } };
  auto isSupported = [](cm::string_view lang) -> bool {
    return std::find(supportedLangs.begin(), supportedLangs.end(), lang) !=
      supportedLangs.end();
  };

  // If the source has an explicit language, validate and return it.
  std::string language = source.GetOrDetermineLanguage();
  if (!language.empty()) {
    if (!isSupported(language)) {
      return cm::nullopt;
    }
    return cm::optional<std::string>(std::move(language));
  }

  /*
    Compute and cache the default language for unlanguaged headers.
    The lattice join is run once per file set, not once per header.
    Lattice:   OBJCXX
              /      \
            CXX      OBJC
              \      /
                 C
  */
  if (!defaultLanguage) {
    std::set<std::string> langs;
    for (AllConfigSource const& tgtSource : this->GetAllConfigSources()) {
      std::string const& lang = tgtSource.Source->GetOrDetermineLanguage();
      if (isSupported(lang)) {
        langs.insert(lang);
      }
    }
    if (langs.empty()) {
      std::vector<std::string> languagesVector;
      this->GlobalGenerator->GetEnabledLanguages(languagesVector);
      for (std::string const& lang : languagesVector) {
        if (isSupported(lang)) {
          langs.insert(lang);
        }
      }
    }

    cm::optional<std::string> resolved;
    if (langs.count("OBJCXX") || (langs.count("CXX") && langs.count("OBJC"))) {
      resolved = "OBJCXX"; // promote
    } else if (langs.count("CXX")) {
      resolved = "CXX";
    } else if (langs.count("OBJC")) {
      resolved = "OBJC";
    } else if (langs.count("C")) {
      resolved = "C";
    }
    defaultLanguage = resolved;
  }

  return *defaultLanguage;
}

cm::optional<std::string> cmGeneratorTarget::GenerateStubForLanguage(
  std::string const& language, std::string const& headerFilename,
  std::string const& verifyTargetName, cmSourceFile& source) const
{
  static std::array<std::pair<cm::string_view, cm::string_view>, 4> const
    langToExt = { {
      { "C", ".c" },
      { "CXX", ".cxx" },
      { "OBJC", ".m" },
      { "OBJCXX", ".mm" },
    } };

  // NOLINTNEXTLINE(readability-qualified-auto)
  auto const it =
    std::find_if(langToExt.begin(), langToExt.end(),
                 [&](std::pair<cm::string_view, cm::string_view> const& p) {
                   return p.first == language;
                 });
  if (it == langToExt.end()) {
    return cm::nullopt;
  }

  std::string filename =
    cmStrCat(this->LocalGenerator->GetCurrentBinaryDirectory(), '/',
             verifyTargetName, '/', headerFilename, it->second);

  cmSourceFile* verificationSource =
    this->Makefile->GetOrCreateSource(filename);
  source.SetSpecialSourceType(
    cmSourceFile::SpecialSourceType::HeaderSetVerificationSource);
  verificationSource->SetProperty("LANGUAGE", language);

  cmSystemTools::MakeDirectory(cmSystemTools::GetFilenamePath(filename));

  cmGeneratedFileStream fout(filename);
  fout.SetCopyIfDifferent(true);
  // The IWYU "associated" pragma tells include-what-you-use to
  // consider the headerFile as part of the entire language
  // unit within include-what-you-use and as a result allows
  // one to get IWYU advice for headers.
  // Also suppress clang-tidy include checks in generated code.
  fout
    << "/* NOLINTNEXTLINE(misc-header-include-cycle,misc-include-cleaner) */\n"
    << "#include <" << headerFilename << "> /* IWYU pragma: associated */\n";
  fout.close();

  return cm::optional<std::string>(std::move(filename));
}
