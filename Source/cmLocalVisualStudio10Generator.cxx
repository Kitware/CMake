/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmLocalVisualStudio10Generator.h"

#include <cmext/string_view>

#include <cm3p/expat.h>

#include "cmGlobalGenerator.h"
#include "cmGlobalVisualStudio10Generator.h"
#include "cmGlobalVisualStudioGenerator.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmVisualStudio10TargetGenerator.h"
#include "cmXMLParser.h"
#include "cmake.h"

class cmGeneratorTarget;

class cmVS10XMLParser : public cmXMLParser
{
public:
  void EndElement(std::string const& /* name */) override {}
  void CharacterDataHandler(char const* data, int length) override
  {
    if (this->DoGUID) {
      if (data[0] == '{') {
        // remove surrounding curly brackets
        this->GUID.assign(data + 1, length - 2);
      } else {
        this->GUID.assign(data, length);
      }
      this->DoGUID = false;
    }
  }
  void StartElement(std::string const& name, char const**) override
  {
    // once the GUID is found do nothing
    if (!this->GUID.empty()) {
      return;
    }
    if (name == "ProjectGUID"_s || name == "ProjectGuid"_s) {
      this->DoGUID = true;
    }
  }
  int InitializeParser() override
  {
    this->DoGUID = false;
    int ret = cmXMLParser::InitializeParser();
    if (ret == 0) {
      return ret;
    }
    // visual studio projects have a strange encoding, but it is
    // really utf-8
    XML_SetEncoding(static_cast<XML_Parser>(this->Parser), "utf-8");
    return 1;
  }
  std::string GUID;
  bool DoGUID;
};

cmLocalVisualStudio10Generator::cmLocalVisualStudio10Generator(
  cmGlobalGenerator* gg, cmMakefile* mf)
  : cmLocalVisualStudio7Generator(gg, mf)
{
}

cmLocalVisualStudio10Generator::~cmLocalVisualStudio10Generator() = default;

void cmLocalVisualStudio10Generator::GenerateTarget(cmGeneratorTarget* target)
{
  if (static_cast<cmGlobalVisualStudioGenerator*>(this->GlobalGenerator)
        ->TargetIsFortranOnly(target)) {
    this->cmLocalVisualStudio7Generator::GenerateTarget(target);
  } else {
    cmVisualStudio10TargetGenerator tg(
      target,
      static_cast<cmGlobalVisualStudio10Generator*>(
        this->GetGlobalGenerator()));
    tg.Generate();
  }
}

void cmLocalVisualStudio10Generator::ReadAndStoreExternalGUID(
  std::string const& name, char const* path)
{
  cmVS10XMLParser parser;
  parser.ParseFile(path);

  // if we can not find a GUID then we will generate one later
  if (parser.GUID.empty()) {
    return;
  }

  std::string guidStoreName = cmStrCat(name, "_GUID_CMAKE");
  // save the GUID in the cache
  this->GlobalGenerator->GetCMakeInstance()->AddCacheEntry(
    guidStoreName, parser.GUID, "Stored GUID", cmStateEnums::INTERNAL);
}

char const* cmLocalVisualStudio10Generator::ReportErrorLabel() const
{
  return ":VCEnd";
}
