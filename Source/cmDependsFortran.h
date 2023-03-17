/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "cmDepends.h"

class cmDependsFortranInternals;
class cmFortranSourceInfo;
class cmLocalUnixMakefileGenerator3;

/** \class cmDependsFortran
 * \brief Dependency scanner for Fortran object files.
 */
class cmDependsFortran : public cmDepends
{
public:
  /** Checking instances need to know the build directory name and the
      relative path from the build directory to the target file.  */
  cmDependsFortran();

  /** Scanning need to know the build directory name, the relative
      path from the build directory to the target file, the source
      file from which to start scanning, the include file search
      path, and the target directory.  */
  cmDependsFortran(cmLocalUnixMakefileGenerator3* lg);

  /** Virtual destructor to cleanup subclasses properly.  */
  ~cmDependsFortran() override;

  cmDependsFortran(cmDependsFortran const&) = delete;
  cmDependsFortran& operator=(cmDependsFortran const&) = delete;

  /** Callback from build system after a .mod file has been generated
      by a Fortran90 compiler to copy the .mod file to the
      corresponding stamp file.  */
  static bool CopyModule(const std::vector<std::string>& args);

  /** Determine if a mod file and the corresponding mod.stamp file
      are representing  different module information. */
  static bool ModulesDiffer(const std::string& modFile,
                            const std::string& stampFile,
                            const std::string& compilerId);

protected:
  // Finalize the dependency information for the target.
  bool Finalize(std::ostream& makeDepends,
                std::ostream& internalDepends) override;

  // Find all the modules required by the target.
  bool LocateModules();
  void MatchLocalModules();
  void MatchRemoteModules(std::istream& fin, const std::string& stampDir);
  void ConsiderModule(const std::string& name, const std::string& stampDir);
  bool FindModule(std::string const& name, std::string& module);

  // Implement writing/checking methods required by superclass.
  bool WriteDependencies(const std::set<std::string>& sources,
                         const std::string& file, std::ostream& makeDepends,
                         std::ostream& internalDepends) override;

  // Actually write the dependencies to the streams.
  bool WriteDependenciesReal(std::string const& obj,
                             cmFortranSourceInfo const& info,
                             std::string const& mod_dir,
                             std::string const& stamp_dir,
                             std::ostream& makeDepends,
                             std::ostream& internalDepends,
                             bool buildingIntrinsics);

  // The source file from which to start scanning.
  std::string SourceFile;

  std::string CompilerId;
  std::string SModSep;
  std::string SModExt;

  std::set<std::string> PPDefinitions;

  // Internal implementation details.
  std::unique_ptr<cmDependsFortranInternals> Internal;
};
