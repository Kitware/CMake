/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>
#include <vector>

class cmLocalGenerator;
class cmXCodeObject;
class cmXMLWriter;

/** \class cmXCodeScheme
 * \brief Write shared schemes for native targets in Xcode project.
 */
class cmXCodeScheme
{
public:
  using TestObjects = std::vector<const cmXCodeObject*>;

  cmXCodeScheme(cmLocalGenerator* lg, cmXCodeObject* xcObj, TestObjects tests,
                const std::vector<std::string>& configList,
                unsigned int xcVersion);

  void WriteXCodeSharedScheme(const std::string& xcProjDir,
                              const std::string& container);

private:
  cmLocalGenerator* const LocalGenerator;
  const cmXCodeObject* const Target;
  const TestObjects Tests;
  const std::string& TargetName;
  const std::vector<std::string>& ConfigList;
  const unsigned int XcodeVersion;

  void WriteXCodeXCScheme(std::ostream& fout, const std::string& container);

  void WriteBuildAction(cmXMLWriter& xout, const std::string& container);
  void WriteTestAction(cmXMLWriter& xout, const std::string& configuration,
                       const std::string& container);
  void WriteLaunchAction(cmXMLWriter& xout, const std::string& configuration,
                         const std::string& container);

  bool WriteLaunchActionAttribute(cmXMLWriter& xout,
                                  const std::string& attrName,
                                  const std::string& varName);

  bool WriteLaunchActionBooleanAttribute(cmXMLWriter& xout,
                                         const std::string& attrName,
                                         const std::string& varName,
                                         bool defaultValue);

  bool WriteLaunchActionAdditionalOption(cmXMLWriter& xout,
                                         const std::string& attrName,
                                         const std::string& value,
                                         const std::string& varName);

  void WriteProfileAction(cmXMLWriter& xout, const std::string& configuration);
  void WriteAnalyzeAction(cmXMLWriter& xout, const std::string& configuration);
  void WriteArchiveAction(cmXMLWriter& xout, const std::string& configuration);

  void WriteBuildableReference(cmXMLWriter& xout, const cmXCodeObject* xcObj,
                               const std::string& container);

  void WriteCustomWorkingDirectory(cmXMLWriter& xout,
                                   const std::string& configuration);

  std::string WriteVersionString();
  std::string FindConfiguration(const std::string& name);

  bool IsTestable() const;

  static bool IsExecutable(const cmXCodeObject* target);
};
