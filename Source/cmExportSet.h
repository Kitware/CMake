/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <memory>
#include <string>
#include <vector>

class cmInstallExportGenerator;
class cmLocalGenerator;
class cmTargetExport;

/// A set of targets that were installed with the same EXPORT parameter.
class cmExportSet
{
public:
  /// Construct an empty export set named \a name
  cmExportSet(std::string name);
  /// Destructor
  ~cmExportSet();

  cmExportSet(const cmExportSet&) = delete;
  cmExportSet& operator=(const cmExportSet&) = delete;

  bool Compute(cmLocalGenerator* lg);

  void AddTargetExport(std::unique_ptr<cmTargetExport> tgt);

  void AddInstallation(cmInstallExportGenerator const* installation);

  std::string const& GetName() const { return this->Name; }

  std::vector<std::unique_ptr<cmTargetExport>> const& GetTargetExports() const
  {
    return this->TargetExports;
  }

  std::vector<cmInstallExportGenerator const*> const* GetInstallations() const
  {
    return &this->Installations;
  }

private:
  std::vector<std::unique_ptr<cmTargetExport>> TargetExports;
  std::string Name;
  std::vector<cmInstallExportGenerator const*> Installations;
};

/// A name -> cmExportSet map with overloaded operator[].
class cmExportSetMap : public std::map<std::string, cmExportSet>
{
public:
  /** \brief Overloaded operator[].
   *
   * The operator is overloaded because cmExportSet has no default constructor:
   * we do not want unnamed export sets.
   */
  cmExportSet& operator[](const std::string& name);
};
