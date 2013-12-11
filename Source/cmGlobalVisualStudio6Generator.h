/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalVisualStudio6Generator_h
#define cmGlobalVisualStudio6Generator_h

#include "cmGlobalVisualStudioGenerator.h"
#include "cmGlobalGeneratorFactory.h"

class cmTarget;

/** \class cmGlobalVisualStudio6Generator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalVisualStudio6Generator manages UNIX build process for a tree
 */
class cmGlobalVisualStudio6Generator : public cmGlobalVisualStudioGenerator
{
public:
  cmGlobalVisualStudio6Generator();
  static cmGlobalGeneratorFactory* NewFactory() {
    return new cmGlobalGeneratorSimpleFactory
      <cmGlobalVisualStudio6Generator>(); }

  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalVisualStudio6Generator::GetActualName();}
  static const char* GetActualName() {return "Visual Studio 6";}

  /** Get the documentation entry for this generator.  */
  static void GetDocumentation(cmDocumentationEntry& entry);

  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.
   */
  virtual void EnableLanguage(std::vector<std::string>const& languages,
                              cmMakefile *, bool optional);

  /**
   * Try running cmake and building a file. This is used for dynalically
   * loaded commands, not as part of the usual build process.
   */
  virtual void GenerateBuildCommand(
    std::vector<std::string>& makeCommand,
    const char* makeProgram,
    const char* projectName,
    const char* projectDir,
    const char* targetName,
    const char* config,
    bool fast,
    std::vector<std::string> const& makeOptions = std::vector<std::string>()
    );

  /**
   * Generate the all required files for building this project/tree. This
   * basically creates a series of LocalGenerators for each directory and
   * requests that they Generate.
   */
  virtual void Generate();

  /**
   * Generate the DSW workspace file.
   */
  virtual void OutputDSWFile();
  virtual void OutputDSWFile(cmLocalGenerator* root,
                             std::vector<cmLocalGenerator*>& generators);
  virtual void WriteDSWFile(std::ostream& fout,
                            cmLocalGenerator* root,
                            std::vector<cmLocalGenerator*>& generators);

  /** Append the subdirectory for the given configuration.  */
  virtual void AppendDirectoryForConfig(const char* prefix,
                                        const char* config,
                                        const char* suffix,
                                        std::string& dir);

  ///! What is the configurations directory variable called?
  virtual const char* GetCMakeCFGIntDir() const { return "$(IntDir)"; }

  virtual void FindMakeProgram(cmMakefile*);

protected:
  virtual const char* GetIDEVersion() { return "6.0"; }
private:
  virtual std::string GetVSMakeProgram() { return this->GetMSDevCommand(); }
  void GenerateConfigurations(cmMakefile* mf);
  void WriteDSWFile(std::ostream& fout);
  void WriteDSWHeader(std::ostream& fout);
  void WriteProject(std::ostream& fout,
                    const char* name, const char* path, cmTarget const& t);
  void WriteExternalProject(std::ostream& fout,
                            const char* name, const char* path,
                            const std::set<cmStdString>& dependencies);
  void WriteDSWFooter(std::ostream& fout);
  virtual std::string WriteUtilityDepend(cmTarget const* target);
  std::string MSDevCommand;
  bool MSDevCommandInitialized;
  std::string const& GetMSDevCommand();
  std::string FindMSDevCommand();
};

#endif
