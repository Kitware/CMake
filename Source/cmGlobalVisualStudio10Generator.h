/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalVisualStudio10Generator_h
#define cmGlobalVisualStudio10Generator_h

#include "cmGlobalVisualStudio8Generator.h"


/** \class cmGlobalVisualStudio10Generator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalVisualStudio10Generator manages UNIX build process for a tree
 */
class cmGlobalVisualStudio10Generator :
  public cmGlobalVisualStudio8Generator
{
public:
  cmGlobalVisualStudio10Generator(const char* name,
    const char* architectureId, const char* additionalPlatformDefinition);
  static cmGlobalGeneratorFactory* NewFactory();

  virtual bool SetGeneratorToolset(std::string const& ts);

  virtual std::string
  GenerateBuildCommand(const char* makeProgram,
                       const char *projectName, const char *projectDir,
                       const char* additionalOptions, const char *targetName,
                       const char* config, bool ignoreErrors, bool);

  virtual void AddPlatformDefinitions(cmMakefile* mf);

  ///! create the correct local generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  virtual void Generate();

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.
   */
  virtual void EnableLanguage(std::vector<std::string>const& languages,
                              cmMakefile *, bool optional);
  virtual void WriteSLNHeader(std::ostream& fout);

  /** Is the installed VS an Express edition?  */
  bool IsExpressEdition() const { return this->ExpressEdition; }

  /** Is the Microsoft Assembler enabled?  */
  bool IsMasmEnabled() const { return this->MasmEnabled; }

  /** The toolset name for the target platform.  */
  const char* GetPlatformToolset();

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
  virtual const char* GetCMakeCFGIntDir() const
    { return "$(Configuration)";}
  bool Find64BitTools(cmMakefile* mf);

  /** Generate an <output>.rule file path for a given command output.  */
  virtual std::string GenerateRuleFile(std::string const& output) const;

  void PathTooLong(cmTarget* target, cmSourceFile* sf,
                   std::string const& sfRel);
protected:
  virtual const char* GetIDEVersion() { return "10.0"; }

  std::string PlatformToolset;
  bool ExpressEdition;
  bool MasmEnabled;

  bool UseFolderProperty();

private:
  class Factory;
  struct LongestSourcePath
  {
    LongestSourcePath(): Length(0), Target(0), SourceFile(0) {}
    size_t Length;
    cmTarget* Target;
    cmSourceFile* SourceFile;
    std::string SourceRel;
  };
  LongestSourcePath LongestSource;
};
#endif
