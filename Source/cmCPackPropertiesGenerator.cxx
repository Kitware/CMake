#include "cmCPackPropertiesGenerator.h"

#include <map>
#include <memory>
#include <ostream>

#include "cmGeneratorExpression.h"
#include "cmInstalledFile.h"

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
  std::ostream& os, std::string const& config, Indent indent)
{
  std::string const& expandedFileName =
    this->InstalledFile.GetNameExpression().Evaluate(this->LG, config);

  cmInstalledFile::PropertyMapType const& properties =
    this->InstalledFile.GetProperties();

  for (cmInstalledFile::PropertyMapType::value_type const& i : properties) {
    std::string const& name = i.first;
    cmInstalledFile::Property const& property = i.second;

    os << indent << "set_property(INSTALL "
       << cmScriptGenerator::Quote(expandedFileName) << " PROPERTY "
       << cmScriptGenerator::Quote(name);

    for (auto const& j : property.ValueExpressions) {
      os << ' ' << cmScriptGenerator::Quote(j->Evaluate(this->LG, config));
    }

    os << ")\n";
  }
}
