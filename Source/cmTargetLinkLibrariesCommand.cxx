/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTargetLinkLibrariesCommand.h"

#include <cstring>
#include <memory>
#include <sstream>
#include <unordered_set>
#include <utility>

#include "cmExecutionStatus.h"
#include "cmGeneratorExpression.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetLinkLibraryType.h"
#include "cmake.h"

namespace {

enum ProcessingState
{
  ProcessingLinkLibraries,
  ProcessingPlainLinkInterface,
  ProcessingKeywordLinkInterface,
  ProcessingPlainPublicInterface,
  ProcessingKeywordPublicInterface,
  ProcessingPlainPrivateInterface,
  ProcessingKeywordPrivateInterface
};

const char* LinkLibraryTypeNames[3] = { "general", "debug", "optimized" };

struct TLL
{
  cmMakefile& Makefile;
  cmTarget* Target;
  bool WarnRemoteInterface = false;
  bool RejectRemoteLinking = false;
  bool EncodeRemoteReference = false;
  std::string DirectoryId;
  std::unordered_set<std::string> Props;

  TLL(cmMakefile& mf, cmTarget* target);
  ~TLL();

  bool HandleLibrary(ProcessingState currentProcessingState,
                     const std::string& lib, cmTargetLinkLibraryType llt);
  void AppendProperty(std::string const& prop, std::string const& value);
  void AffectsProperty(std::string const& prop);
};

} // namespace

static void LinkLibraryTypeSpecifierWarning(cmMakefile& mf, int left,
                                            int right);

bool cmTargetLinkLibrariesCommand(std::vector<std::string> const& args,
                                  cmExecutionStatus& status)
{
  // Must have at least one argument.
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();

  // Alias targets cannot be on the LHS of this command.
  if (mf.IsAlias(args[0])) {
    status.SetError("can not be used on an ALIAS target.");
    return false;
  }

  // Lookup the target for which libraries are specified.
  cmTarget* target =
    mf.GetCMakeInstance()->GetGlobalGenerator()->FindTarget(args[0]);
  if (!target) {
    for (const auto& importedTarget : mf.GetOwnedImportedTargets()) {
      if (importedTarget->GetName() == args[0]) {
        target = importedTarget.get();
        break;
      }
    }
  }
  if (!target) {
    MessageType t = MessageType::FATAL_ERROR; // fail by default
    std::ostringstream e;
    e << "Cannot specify link libraries for target \"" << args[0] << "\" "
      << "which is not built by this project.";
    // The bad target is the only argument. Check how policy CMP0016 is set,
    // and accept, warn or fail respectively:
    if (args.size() < 2) {
      switch (mf.GetPolicyStatus(cmPolicies::CMP0016)) {
        case cmPolicies::WARN:
          t = MessageType::AUTHOR_WARNING;
          // Print the warning.
          e << "\n"
            << "CMake does not support this but it used to work accidentally "
            << "and is being allowed for compatibility."
            << "\n"
            << cmPolicies::GetPolicyWarning(cmPolicies::CMP0016);
          break;
        case cmPolicies::OLD: // OLD behavior does not warn.
          t = MessageType::MESSAGE;
          break;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
          e << "\n" << cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0016);
          break;
        case cmPolicies::NEW: // NEW behavior prints the error.
          break;
      }
    }
    // Now actually print the message.
    switch (t) {
      case MessageType::AUTHOR_WARNING:
        mf.IssueMessage(MessageType::AUTHOR_WARNING, e.str());
        break;
      case MessageType::FATAL_ERROR:
        mf.IssueMessage(MessageType::FATAL_ERROR, e.str());
        cmSystemTools::SetFatalErrorOccured();
        break;
      default:
        break;
    }
    return true;
  }

  // Having a UTILITY library on the LHS is a bug.
  if (target->GetType() == cmStateEnums::UTILITY) {
    std::ostringstream e;
    const char* modal = nullptr;
    MessageType messageType = MessageType::AUTHOR_WARNING;
    switch (mf.GetPolicyStatus(cmPolicies::CMP0039)) {
      case cmPolicies::WARN:
        e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0039) << "\n";
        modal = "should";
      case cmPolicies::OLD:
        break;
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::NEW:
        modal = "must";
        messageType = MessageType::FATAL_ERROR;
    }
    if (modal) {
      e << "Utility target \"" << target->GetName() << "\" " << modal
        << " not be used as the target of a target_link_libraries call.";
      mf.IssueMessage(messageType, e.str());
      if (messageType == MessageType::FATAL_ERROR) {
        return false;
      }
    }
  }

  // But we might not have any libs after variable expansion.
  if (args.size() < 2) {
    return true;
  }

  TLL tll(mf, target);

  // Keep track of link configuration specifiers.
  cmTargetLinkLibraryType llt = GENERAL_LibraryType;
  bool haveLLT = false;

  // Start with primary linking and switch to link interface
  // specification if the keyword is encountered as the first argument.
  ProcessingState currentProcessingState = ProcessingLinkLibraries;

  // Add libraries, note that there is an optional prefix
  // of debug and optimized that can be used.
  for (unsigned int i = 1; i < args.size(); ++i) {
    if (args[i] == "LINK_INTERFACE_LIBRARIES") {
      currentProcessingState = ProcessingPlainLinkInterface;
      if (i != 1) {
        mf.IssueMessage(
          MessageType::FATAL_ERROR,
          "The LINK_INTERFACE_LIBRARIES option must appear as the second "
          "argument, just after the target name.");
        return true;
      }
    } else if (args[i] == "INTERFACE") {
      if (i != 1 &&
          currentProcessingState != ProcessingKeywordPrivateInterface &&
          currentProcessingState != ProcessingKeywordPublicInterface &&
          currentProcessingState != ProcessingKeywordLinkInterface) {
        mf.IssueMessage(
          MessageType::FATAL_ERROR,
          "The INTERFACE, PUBLIC or PRIVATE option must appear as the second "
          "argument, just after the target name.");
        return true;
      }
      currentProcessingState = ProcessingKeywordLinkInterface;
    } else if (args[i] == "LINK_PUBLIC") {
      if (i != 1 &&
          currentProcessingState != ProcessingPlainPrivateInterface &&
          currentProcessingState != ProcessingPlainPublicInterface) {
        mf.IssueMessage(
          MessageType::FATAL_ERROR,
          "The LINK_PUBLIC or LINK_PRIVATE option must appear as the second "
          "argument, just after the target name.");
        return true;
      }
      currentProcessingState = ProcessingPlainPublicInterface;
    } else if (args[i] == "PUBLIC") {
      if (i != 1 &&
          currentProcessingState != ProcessingKeywordPrivateInterface &&
          currentProcessingState != ProcessingKeywordPublicInterface &&
          currentProcessingState != ProcessingKeywordLinkInterface) {
        mf.IssueMessage(
          MessageType::FATAL_ERROR,
          "The INTERFACE, PUBLIC or PRIVATE option must appear as the second "
          "argument, just after the target name.");
        return true;
      }
      currentProcessingState = ProcessingKeywordPublicInterface;
    } else if (args[i] == "LINK_PRIVATE") {
      if (i != 1 && currentProcessingState != ProcessingPlainPublicInterface &&
          currentProcessingState != ProcessingPlainPrivateInterface) {
        mf.IssueMessage(
          MessageType::FATAL_ERROR,
          "The LINK_PUBLIC or LINK_PRIVATE option must appear as the second "
          "argument, just after the target name.");
        return true;
      }
      currentProcessingState = ProcessingPlainPrivateInterface;
    } else if (args[i] == "PRIVATE") {
      if (i != 1 &&
          currentProcessingState != ProcessingKeywordPrivateInterface &&
          currentProcessingState != ProcessingKeywordPublicInterface &&
          currentProcessingState != ProcessingKeywordLinkInterface) {
        mf.IssueMessage(
          MessageType::FATAL_ERROR,
          "The INTERFACE, PUBLIC or PRIVATE option must appear as the second "
          "argument, just after the target name.");
        return true;
      }
      currentProcessingState = ProcessingKeywordPrivateInterface;
    } else if (args[i] == "debug") {
      if (haveLLT) {
        LinkLibraryTypeSpecifierWarning(mf, llt, DEBUG_LibraryType);
      }
      llt = DEBUG_LibraryType;
      haveLLT = true;
    } else if (args[i] == "optimized") {
      if (haveLLT) {
        LinkLibraryTypeSpecifierWarning(mf, llt, OPTIMIZED_LibraryType);
      }
      llt = OPTIMIZED_LibraryType;
      haveLLT = true;
    } else if (args[i] == "general") {
      if (haveLLT) {
        LinkLibraryTypeSpecifierWarning(mf, llt, GENERAL_LibraryType);
      }
      llt = GENERAL_LibraryType;
      haveLLT = true;
    } else if (haveLLT) {
      // The link type was specified by the previous argument.
      haveLLT = false;
      if (!tll.HandleLibrary(currentProcessingState, args[i], llt)) {
        return false;
      }
    } else {
      // Lookup old-style cache entry if type is unspecified.  So if you
      // do a target_link_libraries(foo optimized bar) it will stay optimized
      // and not use the lookup.  As there may be the case where someone has
      // specified that a library is both debug and optimized.  (this check is
      // only there for backwards compatibility when mixing projects built
      // with old versions of CMake and new)
      llt = GENERAL_LibraryType;
      std::string linkType = cmStrCat(args[0], "_LINK_TYPE");
      const char* linkTypeString = mf.GetDefinition(linkType);
      if (linkTypeString) {
        if (strcmp(linkTypeString, "debug") == 0) {
          llt = DEBUG_LibraryType;
        }
        if (strcmp(linkTypeString, "optimized") == 0) {
          llt = OPTIMIZED_LibraryType;
        }
      }
      if (!tll.HandleLibrary(currentProcessingState, args[i], llt)) {
        return false;
      }
    }
  }

  // Make sure the last argument was not a library type specifier.
  if (haveLLT) {
    mf.IssueMessage(MessageType::FATAL_ERROR,
                    cmStrCat("The \"", LinkLibraryTypeNames[llt],
                             "\" argument must be followed by a library."));
    cmSystemTools::SetFatalErrorOccured();
  }

  const cmPolicies::PolicyStatus policy22Status =
    target->GetPolicyStatusCMP0022();

  // If any of the LINK_ options were given, make sure the
  // LINK_INTERFACE_LIBRARIES target property exists.
  // Use of any of the new keywords implies awareness of
  // this property. And if no libraries are named, it should
  // result in an empty link interface.
  if ((policy22Status == cmPolicies::OLD ||
       policy22Status == cmPolicies::WARN) &&
      currentProcessingState != ProcessingLinkLibraries &&
      !target->GetProperty("LINK_INTERFACE_LIBRARIES")) {
    target->SetProperty("LINK_INTERFACE_LIBRARIES", "");
  }

  return true;
}

static void LinkLibraryTypeSpecifierWarning(cmMakefile& mf, int left,
                                            int right)
{
  mf.IssueMessage(
    MessageType::AUTHOR_WARNING,
    cmStrCat(
      "Link library type specifier \"", LinkLibraryTypeNames[left],
      "\" is followed by specifier \"", LinkLibraryTypeNames[right],
      "\" instead of a library name.  The first specifier will be ignored."));
}

namespace {

TLL::TLL(cmMakefile& mf, cmTarget* target)
  : Makefile(mf)
  , Target(target)
{
  if (&this->Makefile != this->Target->GetMakefile()) {
    // The LHS target was created in another directory.
    switch (this->Makefile.GetPolicyStatus(cmPolicies::CMP0079)) {
      case cmPolicies::WARN:
        this->WarnRemoteInterface = true;
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        this->RejectRemoteLinking = true;
        break;
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::NEW:
        this->EncodeRemoteReference = true;
        break;
    }
  }
  if (this->EncodeRemoteReference) {
    cmDirectoryId const dirId = this->Makefile.GetDirectoryId();
    this->DirectoryId = cmStrCat(CMAKE_DIRECTORY_ID_SEP, dirId.String);
  }
}

bool TLL::HandleLibrary(ProcessingState currentProcessingState,
                        const std::string& lib, cmTargetLinkLibraryType llt)
{
  if (this->Target->GetType() == cmStateEnums::INTERFACE_LIBRARY &&
      currentProcessingState != ProcessingKeywordLinkInterface) {
    this->Makefile.IssueMessage(
      MessageType::FATAL_ERROR,
      "INTERFACE library can only be used with the INTERFACE keyword of "
      "target_link_libraries");
    return false;
  }
  if (this->Target->IsImported() &&
      currentProcessingState != ProcessingKeywordLinkInterface) {
    this->Makefile.IssueMessage(
      MessageType::FATAL_ERROR,
      "IMPORTED library can only be used with the INTERFACE keyword of "
      "target_link_libraries");
    return false;
  }

  cmTarget::TLLSignature sig =
    (currentProcessingState == ProcessingPlainPrivateInterface ||
     currentProcessingState == ProcessingPlainPublicInterface ||
     currentProcessingState == ProcessingKeywordPrivateInterface ||
     currentProcessingState == ProcessingKeywordPublicInterface ||
     currentProcessingState == ProcessingKeywordLinkInterface)
    ? cmTarget::KeywordTLLSignature
    : cmTarget::PlainTLLSignature;
  if (!this->Target->PushTLLCommandTrace(
        sig, this->Makefile.GetExecutionContext())) {
    std::ostringstream e;
    const char* modal = nullptr;
    MessageType messageType = MessageType::AUTHOR_WARNING;
    switch (this->Makefile.GetPolicyStatus(cmPolicies::CMP0023)) {
      case cmPolicies::WARN:
        e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0023) << "\n";
        modal = "should";
      case cmPolicies::OLD:
        break;
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::NEW:
        modal = "must";
        messageType = MessageType::FATAL_ERROR;
    }

    if (modal) {
      // If the sig is a keyword form and there is a conflict, the existing
      // form must be the plain form.
      const char* existingSig =
        (sig == cmTarget::KeywordTLLSignature ? "plain" : "keyword");
      e << "The " << existingSig
        << " signature for target_link_libraries has "
           "already been used with the target \""
        << this->Target->GetName()
        << "\".  All uses of target_link_libraries with a target " << modal
        << " be either all-keyword or all-plain.\n";
      this->Target->GetTllSignatureTraces(e,
                                          sig == cmTarget::KeywordTLLSignature
                                            ? cmTarget::PlainTLLSignature
                                            : cmTarget::KeywordTLLSignature);
      this->Makefile.IssueMessage(messageType, e.str());
      if (messageType == MessageType::FATAL_ERROR) {
        return false;
      }
    }
  }

  // Handle normal case where the command was called with another keyword than
  // INTERFACE / LINK_INTERFACE_LIBRARIES or none at all. (The "LINK_LIBRARIES"
  // property of the target on the LHS shall be populated.)
  if (currentProcessingState != ProcessingKeywordLinkInterface &&
      currentProcessingState != ProcessingPlainLinkInterface) {

    if (this->RejectRemoteLinking) {
      this->Makefile.IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("Attempt to add link library \"", lib, "\" to target \"",
                 this->Target->GetName(),
                 "\" which is not built in this "
                 "directory.\nThis is allowed only when policy CMP0079 "
                 "is set to NEW."));
      return false;
    }

    cmTarget* tgt = this->Makefile.GetGlobalGenerator()->FindTarget(lib);

    if (tgt && (tgt->GetType() != cmStateEnums::STATIC_LIBRARY) &&
        (tgt->GetType() != cmStateEnums::SHARED_LIBRARY) &&
        (tgt->GetType() != cmStateEnums::UNKNOWN_LIBRARY) &&
        (tgt->GetType() != cmStateEnums::OBJECT_LIBRARY) &&
        (tgt->GetType() != cmStateEnums::INTERFACE_LIBRARY) &&
        !tgt->IsExecutableWithExports()) {
      this->Makefile.IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat(
          "Target \"", lib, "\" of type ",
          cmState::GetTargetTypeName(tgt->GetType()),
          " may not be linked into another target. One may link only to "
          "INTERFACE, OBJECT, STATIC or SHARED libraries, or to ",
          "executables with the ENABLE_EXPORTS property set."));
    }

    this->AffectsProperty("LINK_LIBRARIES");
    this->Target->AddLinkLibrary(this->Makefile, lib, llt);
  }

  if (this->WarnRemoteInterface) {
    this->Makefile.IssueMessage(
      MessageType::AUTHOR_WARNING,
      cmStrCat(
        cmPolicies::GetPolicyWarning(cmPolicies::CMP0079), "\nTarget\n  ",
        this->Target->GetName(),
        "\nis not created in this "
        "directory.  For compatibility with older versions of CMake, link "
        "library\n  ",
        lib,
        "\nwill be looked up in the directory in which "
        "the target was created rather than in this calling directory."));
  }

  // Handle (additional) case where the command was called with PRIVATE /
  // LINK_PRIVATE and stop its processing. (The "INTERFACE_LINK_LIBRARIES"
  // property of the target on the LHS shall only be populated if it is a
  // STATIC library.)
  if (currentProcessingState == ProcessingKeywordPrivateInterface ||
      currentProcessingState == ProcessingPlainPrivateInterface) {
    if (this->Target->GetType() == cmStateEnums::STATIC_LIBRARY ||
        this->Target->GetType() == cmStateEnums::OBJECT_LIBRARY) {
      std::string configLib =
        this->Target->GetDebugGeneratorExpressions(lib, llt);
      if (cmGeneratorExpression::IsValidTargetName(lib) ||
          cmGeneratorExpression::Find(lib) != std::string::npos) {
        configLib = "$<LINK_ONLY:" + configLib + ">";
      }
      this->AppendProperty("INTERFACE_LINK_LIBRARIES", configLib);
    }
    return true;
  }

  // Handle general case where the command was called with another keyword than
  // PRIVATE / LINK_PRIVATE or none at all. (The "INTERFACE_LINK_LIBRARIES"
  // property of the target on the LHS shall be populated.)
  this->AppendProperty("INTERFACE_LINK_LIBRARIES",
                       this->Target->GetDebugGeneratorExpressions(lib, llt));

  // Stop processing if called without any keyword.
  if (currentProcessingState == ProcessingLinkLibraries) {
    return true;
  }
  // Stop processing if policy CMP0022 is set to NEW.
  const cmPolicies::PolicyStatus policy22Status =
    this->Target->GetPolicyStatusCMP0022();
  if (policy22Status != cmPolicies::OLD &&
      policy22Status != cmPolicies::WARN) {
    return true;
  }
  // Stop processing if called with an INTERFACE library on the LHS.
  if (this->Target->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
    return true;
  }

  // Handle (additional) backward-compatibility case where the command was
  // called with PUBLIC / INTERFACE / LINK_PUBLIC / LINK_INTERFACE_LIBRARIES.
  // (The policy CMP0022 is not set to NEW.)
  {
    // Get the list of configurations considered to be DEBUG.
    std::vector<std::string> debugConfigs =
      this->Makefile.GetCMakeInstance()->GetDebugConfigs();
    std::string prop;

    // Include this library in the link interface for the target.
    if (llt == DEBUG_LibraryType || llt == GENERAL_LibraryType) {
      // Put in the DEBUG configuration interfaces.
      for (std::string const& dc : debugConfigs) {
        prop = cmStrCat("LINK_INTERFACE_LIBRARIES_", dc);
        this->AppendProperty(prop, lib);
      }
    }
    if (llt == OPTIMIZED_LibraryType || llt == GENERAL_LibraryType) {
      // Put in the non-DEBUG configuration interfaces.
      this->AppendProperty("LINK_INTERFACE_LIBRARIES", lib);

      // Make sure the DEBUG configuration interfaces exist so that the
      // general one will not be used as a fall-back.
      for (std::string const& dc : debugConfigs) {
        prop = cmStrCat("LINK_INTERFACE_LIBRARIES_", dc);
        if (!this->Target->GetProperty(prop)) {
          this->Target->SetProperty(prop, "");
        }
      }
    }
  }
  return true;
}

void TLL::AppendProperty(std::string const& prop, std::string const& value)
{
  this->AffectsProperty(prop);
  this->Target->AppendProperty(prop, value);
}

void TLL::AffectsProperty(std::string const& prop)
{
  if (!this->EncodeRemoteReference) {
    return;
  }
  // Add a wrapper to the expression to tell LookupLinkItems to look up
  // names in the caller's directory.
  if (this->Props.insert(prop).second) {
    this->Target->AppendProperty(prop, this->DirectoryId);
  }
}

TLL::~TLL()
{
  for (std::string const& prop : this->Props) {
    this->Target->AppendProperty(prop, CMAKE_DIRECTORY_ID_SEP);
  }
}

} // namespace
