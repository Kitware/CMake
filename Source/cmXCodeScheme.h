/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
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
  using TestObjects = std::vector<cmXCodeObject const*>;

  cmXCodeScheme(cmLocalGenerator* lg, cmXCodeObject* xcObj, TestObjects tests,
                std::vector<std::string> const& configList,
                unsigned int xcVersion);

  void WriteXCodeSharedScheme(std::string const& xcProjDir,
                              std::string const& container);

private:
  cmLocalGenerator* const LocalGenerator;
  cmXCodeObject const* const Target;
  TestObjects const Tests;
  std::string const& TargetName;
  std::vector<std::string> const& ConfigList;
  unsigned int const XcodeVersion;

  void WriteXCodeXCScheme(std::ostream& fout, std::string const& container);

  void WriteBuildAction(cmXMLWriter& xout, std::string const& container);
  void WriteTestAction(cmXMLWriter& xout, std::string const& configuration,
                       std::string const& container);
  void WriteLaunchAction(cmXMLWriter& xout, std::string const& configuration,
                         std::string const& container);

  bool WriteLaunchActionAttribute(cmXMLWriter& xout,
                                  std::string const& attrName,
                                  std::string const& varName);

  bool WriteLaunchActionBooleanAttribute(cmXMLWriter& xout,
                                         std::string const& attrName,
                                         std::string const& varName,
                                         bool defaultValue);

  bool WriteLaunchActionAdditionalOption(cmXMLWriter& xout,
                                         std::string const& attrName,
                                         std::string const& value,
                                         std::string const& varName);

  void WriteProfileAction(cmXMLWriter& xout, std::string const& configuration,
                          std::string const& container);
  void WriteAnalyzeAction(cmXMLWriter& xout, std::string const& configuration);
  void WriteArchiveAction(cmXMLWriter& xout, std::string const& configuration);

  void WriteBuildableProductRunnable(cmXMLWriter& xout,
                                     cmXCodeObject const* xcObj,
                                     std::string const& container);
  void WriteBuildableReference(cmXMLWriter& xout, cmXCodeObject const* xcObj,
                               std::string const& container);

  void WriteCustomWorkingDirectory(cmXMLWriter& xout,
                                   std::string const& configuration);

  void WriteCustomLLDBInitFile(cmXMLWriter& xout,
                               std::string const& configuration);

  std::string WriteVersionString();
  std::string FindConfiguration(std::string const& name);

  bool IsTestable() const;

  static bool IsExecutable(cmXCodeObject const* target);
};
