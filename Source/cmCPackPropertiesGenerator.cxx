#include "cmCPackPropertiesGenerator.h"

#include <map>
#include <ostream>

#include "cmGeneratorExpression.h"
#include "cmInstalledFile.h"
#include "cmOutputConverter.h"

cmCPackPropertiesGenerator::cmCPackPropertiesGenerator(
  cmLocalGenerator* lg, cmInstalledFile const& installedFile,
  std::vector<std::string> const& configurations)
  : cmScriptGenerator("CPACK_BUILD_CONFIG", configurations)
  , LG(lg)
  , InstalledFile(installedFile)
{
  this->ActionsPerConfig = true;
}

void cmCPackPropertiesGenerator::GenerateScriptForConfig(
  std::ostream& os, const std::string& config, Indent indent)
{
  std::string const& expandedFileName =
    this->InstalledFile.GetNameExpression().Evaluate(this->LG, config);

  cmInstalledFile::PropertyMapType const& properties =
    this->InstalledFile.GetProperties();

  for (cmInstalledFile::PropertyMapType::value_type const& i : properties) {
    std::string const& name = i.first;
    cmInstalledFile::Property const& property = i.second;

    os << indent << "set_property(INSTALL "
       << cmOutputConverter::EscapeForCMake(expandedFileName) << " PROPERTY "
       << cmOutputConverter::EscapeForCMake(name);

    for (cmInstalledFile::ExpressionVectorType::value_type const& j :
         property.ValueExpressions) {
      std::string value = j->Evaluate(this->LG, config);
      os << " " << cmOutputConverter::EscapeForCMake(value);
    }

    os << ")\n";
  }
}
