/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cmGeneratorExpression.h"

class cmMakefile;

/** \class cmInstalledFile
 * \brief Represents a file intended for installation.
 *
 * cmInstalledFile represents a file intended for installation.
 */
class cmInstalledFile
{
public:
  using CompiledGeneratorExpressionPtrType =
    std::unique_ptr<cmCompiledGeneratorExpression>;

  using ExpressionVectorType = std::vector<CompiledGeneratorExpressionPtrType>;

  struct Property
  {
    Property();
    ~Property();

    Property(Property const&) = delete;
    Property& operator=(Property const&) = delete;

    ExpressionVectorType ValueExpressions;
  };

  using PropertyMapType = std::map<std::string, Property>;

  cmInstalledFile();

  ~cmInstalledFile();

  cmInstalledFile(cmInstalledFile const&) = delete;
  cmInstalledFile& operator=(cmInstalledFile const&) = delete;

  void RemoveProperty(std::string const& prop);

  void SetProperty(cmMakefile const* mf, std::string const& prop,
                   std::string const& value);

  void AppendProperty(cmMakefile const* mf, std::string const& prop,
                      std::string const& value, bool asString = false);

  bool HasProperty(std::string const& prop) const;

  bool GetProperty(std::string const& prop, std::string& value) const;

  bool GetPropertyAsBool(std::string const& prop) const;

  std::vector<std::string> GetPropertyAsList(std::string const& prop) const;

  void SetName(cmMakefile* mf, std::string const& name);

  std::string const& GetName() const;

  cmCompiledGeneratorExpression const& GetNameExpression() const;

  PropertyMapType const& GetProperties() const { return this->Properties; }

private:
  std::string Name;
  CompiledGeneratorExpressionPtrType NameExpression;
  PropertyMapType Properties;
};
