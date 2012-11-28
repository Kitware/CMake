/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalVisualStudio8Generator_h
#define cmGlobalVisualStudio8Generator_h

#include "cmGlobalVisualStudio71Generator.h"


/** \class cmGlobalVisualStudio8Generator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalVisualStudio8Generator manages UNIX build process for a tree
 */
class cmGlobalVisualStudio8Generator : public cmGlobalVisualStudio71Generator
{
public:
  cmGlobalVisualStudio8Generator(const char* name,
    const char* architectureId, const char* additionalPlatformDefinition);
  static cmGlobalGeneratorFactory* NewFactory();

  ///! Get the name for the generator.
  virtual const char* GetName() const {return this->Name.c_str();}

  const char* GetPlatformName() const;

  /** Get the documentation entry for this generator.  */
  static void GetDocumentation(cmDocumentationEntry& entry);

  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  virtual void AddPlatformDefinitions(cmMakefile* mf);

  /**
   * Override Configure and Generate to add the build-system check
   * target.
   */
  virtual void Configure();
  virtual void Generate();

  /**
   * Where does this version of Visual Studio look for macros for the
   * current user? Returns the empty string if this version of Visual
   * Studio does not implement support for VB macros.
   */
  virtual std::string GetUserMacrosDirectory();

  /**
   * What is the reg key path to "vsmacros" for this version of Visual
   * Studio?
   */
  virtual std::string GetUserMacrosRegKeyBase();

  /** Return true if the target project file should have the option
      LinkLibraryDependencies and link to .sln dependencies. */
  virtual bool NeedLinkLibraryDependencies(cmTarget& target);

  /** Return true if building for Windows CE */
  virtual bool TargetsWindowsCE() const {
    return !this->WindowsCEVersion.empty(); }

protected:
  virtual const char* GetIDEVersion() { return "8.0"; }

  virtual bool VSLinksDependencies() const { return false; }

  void AddCheckTarget();

  static cmIDEFlagTable const* GetExtraFlagTableVS8();
  virtual void WriteSLNHeader(std::ostream& fout);
  virtual void WriteSolutionConfigurations(std::ostream& fout);
  virtual void WriteProjectConfigurations(
    std::ostream& fout, const char* name, cmTarget::TargetType type,
    const std::set<std::string>& configsPartOfDefaultBuild,
    const char* platformMapping = NULL);
  virtual bool ComputeTargetDepends();
  virtual void WriteProjectDepends(std::ostream& fout, const char* name,
                                   const char* path, cmTarget &t);

  std::string Name;
  std::string PlatformName;
  std::string WindowsCEVersion;

private:
  class Factory;
  friend class Factory;
};
#endif
