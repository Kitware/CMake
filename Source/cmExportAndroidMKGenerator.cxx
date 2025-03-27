/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExportAndroidMKGenerator.h"

#include <sstream>
#include <utility>
#include <vector>

#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmGeneratorTarget.h"
#include "cmLinkItem.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"

cmExportAndroidMKGenerator::cmExportAndroidMKGenerator() = default;

cm::string_view cmExportAndroidMKGenerator::GetImportPrefixWithSlash() const
{
  return "$(_IMPORT_PREFIX)/"_s;
}

bool cmExportAndroidMKGenerator::GenerateImportFile(std::ostream& os)
{
  if (!this->AppendMode) {
    // Start with the import file header.
    this->GenerateImportHeaderCode(os);
  }

  // Create all the imported targets.
  std::stringstream mainFileBuffer;
  bool result = this->GenerateMainFile(mainFileBuffer);

  // Write cached import code.
  os << mainFileBuffer.rdbuf();

  return result;
}

void cmExportAndroidMKGenerator::GenerateInterfaceProperties(
  cmGeneratorTarget const* target, std::ostream& os,
  ImportPropertyMap const& properties)
{
  std::string const config =
    (this->Configurations.empty() ? std::string{} : this->Configurations[0]);
  GenerateType const type = this->GetGenerateType();

  bool const newCMP0022Behavior =
    target->GetPolicyStatusCMP0022() != cmPolicies::WARN &&
    target->GetPolicyStatusCMP0022() != cmPolicies::OLD;
  if (!newCMP0022Behavior) {
    std::ostringstream w;
    if (type == BUILD) {
      w << "export(TARGETS ... ANDROID_MK) called with policy CMP0022";
    } else {
      w << "install( EXPORT_ANDROID_MK ...) called with policy CMP0022";
    }
    w << " set to OLD for target " << target->Target->GetName() << ". "
      << "The export will only work with CMP0022 set to NEW.";
    target->Makefile->IssueMessage(MessageType::AUTHOR_WARNING, w.str());
  }
  if (!properties.empty()) {
    os << "LOCAL_CPP_FEATURES := rtti exceptions\n";
    for (auto const& property : properties) {
      if (property.first == "INTERFACE_COMPILE_OPTIONS") {
        os << "LOCAL_CPP_FEATURES += ";
        os << (property.second) << "\n";
      } else if (property.first == "INTERFACE_LINK_LIBRARIES") {
        std::string staticLibs;
        std::string sharedLibs;
        std::string ldlibs;
        cmLinkInterfaceLibraries const* linkIFace =
          target->GetLinkInterfaceLibraries(config, target,
                                            cmGeneratorTarget::UseTo::Link);
        for (cmLinkItem const& item : linkIFace->Libraries) {
          cmGeneratorTarget const* gt = item.Target;
          std::string const& lib = item.AsStr();
          if (gt) {

            if (gt->GetType() == cmStateEnums::SHARED_LIBRARY ||
                gt->GetType() == cmStateEnums::MODULE_LIBRARY) {
              sharedLibs += " " + lib;
            } else {
              staticLibs += " " + lib;
            }
          } else {
            bool relpath = false;
            if (type == INSTALL) {
              relpath = cmHasLiteralPrefix(lib, "../");
            }
            // check for full path or if it already has a -l, or
            // in the case of an install check for relative paths
            // if it is full or a link library then use string directly
            if (cmSystemTools::FileIsFullPath(lib) ||
                cmHasLiteralPrefix(lib, "-l") || relpath) {
              ldlibs += " " + lib;
              // if it is not a path and does not have a -l then add -l
            } else if (!lib.empty()) {
              ldlibs += " -l" + lib;
            }
          }
        }
        if (!sharedLibs.empty()) {
          os << "LOCAL_SHARED_LIBRARIES :=" << sharedLibs << "\n";
        }
        if (!staticLibs.empty()) {
          os << "LOCAL_STATIC_LIBRARIES :=" << staticLibs << "\n";
        }
        if (!ldlibs.empty()) {
          os << "LOCAL_EXPORT_LDLIBS :=" << ldlibs << "\n";
        }
      } else if (property.first == "INTERFACE_INCLUDE_DIRECTORIES") {
        std::string includes = property.second;
        cmList includeList{ includes };
        os << "LOCAL_EXPORT_C_INCLUDES := ";
        std::string end;
        for (std::string const& i : includeList) {
          os << end << i;
          end = "\\\n";
        }
        os << "\n";
      } else if (property.first == "INTERFACE_LINK_OPTIONS") {
        os << "LOCAL_EXPORT_LDFLAGS := ";
        cmList linkFlagsList{ property.second };
        os << linkFlagsList.join(" ") << "\n";
      } else {
        os << "# " << property.first << " " << (property.second) << "\n";
      }
    }
  }

  // Tell the NDK build system if prebuilt static libraries use C++.
  if (target->GetType() == cmStateEnums::STATIC_LIBRARY) {
    cmLinkImplementation const* li =
      target->GetLinkImplementation(config, cmGeneratorTarget::UseTo::Link);
    if (cm::contains(li->Languages, "CXX")) {
      os << "LOCAL_HAS_CPP := true\n";
    }
  }

  switch (target->GetType()) {
    case cmStateEnums::SHARED_LIBRARY:
    case cmStateEnums::MODULE_LIBRARY:
      os << "include $(PREBUILT_SHARED_LIBRARY)\n";
      break;
    case cmStateEnums::STATIC_LIBRARY:
      os << "include $(PREBUILT_STATIC_LIBRARY)\n";
      break;
    case cmStateEnums::EXECUTABLE:
    case cmStateEnums::UTILITY:
    case cmStateEnums::OBJECT_LIBRARY:
    case cmStateEnums::GLOBAL_TARGET:
    case cmStateEnums::INTERFACE_LIBRARY:
    case cmStateEnums::UNKNOWN_LIBRARY:
      break;
  }
  os << "\n";
}
