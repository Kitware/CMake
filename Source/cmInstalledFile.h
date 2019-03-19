/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmInstalledFile_h
#define cmInstalledFile_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmGeneratorExpression.h"

#include <map>
#include <memory> // IWYU pragma: keep
#include <string>
#include <vector>

class cmMakefile;

/** \class cmInstalledFile
 * \brief Represents a file intended for installation.
 *
 * cmInstalledFile represents a file intended for installation.
 */
class cmInstalledFile
{
public:
  typedef std::unique_ptr<cmCompiledGeneratorExpression>
    CompiledGeneratorExpressionPtrType;

  typedef std::vector<cmCompiledGeneratorExpression*> ExpressionVectorType;

  struct Property
  {
    Property();
    ~Property();

    Property(const Property&) = delete;
    Property& operator=(const Property&) = delete;

    ExpressionVectorType ValueExpressions;
  };

  typedef std::map<std::string, Property> PropertyMapType;

  cmInstalledFile();

  ~cmInstalledFile();

  cmInstalledFile(const cmInstalledFile&) = delete;
  cmInstalledFile& operator=(const cmInstalledFile&) = delete;

  void RemoveProperty(const std::string& prop);

  void SetProperty(cmMakefile const* mf, const std::string& prop,
                   const char* value);

  void AppendProperty(cmMakefile const* mf, const std::string& prop,
                      const char* value, bool asString = false);

  bool HasProperty(const std::string& prop) const;

  bool GetProperty(const std::string& prop, std::string& value) const;

  bool GetPropertyAsBool(const std::string& prop) const;

  void GetPropertyAsList(const std::string& prop,
                         std::vector<std::string>& list) const;

  void SetName(cmMakefile* mf, const std::string& name);

  std::string const& GetName() const;

  cmCompiledGeneratorExpression const& GetNameExpression() const;

  PropertyMapType const& GetProperties() const { return this->Properties; }

private:
  std::string Name;
  cmCompiledGeneratorExpression* NameExpression = nullptr;
  PropertyMapType Properties;
};

#endif
