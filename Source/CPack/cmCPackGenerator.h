/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackGenerator_h
#define cmCPackGenerator_h

#include "cmObject.h"
#include <map>
#include <vector>

#include "cmCPackComponentGroup.h" // cmCPackComponent and friends
  // Forward declarations are insufficient since we use them in
  // std::map data members below...

#define cmCPackTypeMacro(class, superclass) \
  cmTypeMacro(class, superclass); \
  static cmCPackGenerator* CreateGenerator() { return new class; }

#define cmCPackLogger(logType, msg) \
  do { \
    cmOStringStream cmCPackLog_msg; \
    cmCPackLog_msg << msg; \
    this->Logger->Log(logType, __FILE__, __LINE__,\
                      cmCPackLog_msg.str().c_str());\
  } while ( 0 )

#ifdef cerr
#  undef cerr
#endif
#define cerr no_cerr_use_cmCPack_Log

#ifdef cout
#  undef cout
#endif
#define cout no_cout_use_cmCPack_Log

class cmMakefile;
class cmCPackLog;

/** \class cmCPackGenerator
 * \brief A superclass of all CPack Generators
 *
 */
class cmCPackGenerator : public cmObject
{
public:
  cmTypeMacro(cmCPackGenerator, cmObject);
  /**
   * If verbose then more informaiton is printed out
   */
  void SetVerbose(bool val) { this->GeneratorVerbose = val; }

  /**
   * Do the actual processing. Subclass has to override it.
   * Return 0 if error.
   */
  virtual int DoPackage();

  /**
   * Initialize generator
   */
  int Initialize(const char* name, cmMakefile* mf);

  /**
   * Construct generator
   */
  cmCPackGenerator();
  virtual ~cmCPackGenerator();

  //! Set and get the options
  void SetOption(const char* op, const char* value);
  void SetOptionIfNotSet(const char* op, const char* value);
  const char* GetOption(const char* op);
  bool IsSet(const char* name) const;

  //! Set all the variables
  int SetCMakeRoot();

  //! Set the logger
  void SetLogger(cmCPackLog* log) { this->Logger = log; }

  //! Display verbose information via logger
  void DisplayVerboseOutput(const char* msg, float progress);
  
  bool ReadListFile(const char* moduleName);

protected:
  int PrepareNames();
  int InstallProject();
  int CleanTemporaryDirectory();
  virtual const char* GetOutputExtension() { return ".cpack"; }
  virtual const char* GetOutputPostfix() { return 0; }
  virtual int CompressFiles(const char* outFileName, const char* toplevel,
    const std::vector<std::string>& files);
  virtual const char* GetInstallPath();
  virtual const char* GetPackagingInstallPrefix();

  virtual std::string FindTemplate(const char* name);
  virtual bool ConfigureFile(const char* inName, const char* outName,
    bool copyOnly = false);
  virtual bool ConfigureString(const std::string& input, std::string& output);
  virtual int InitializeInternal();


  //! Run install commands if specified
  virtual int InstallProjectViaInstallCommands(
    bool setDestDir, const char* tempInstallDirectory);
  virtual int InstallProjectViaInstallScript(
    bool setDestDir, const char* tempInstallDirectory);
  virtual int InstallProjectViaInstalledDirectories(
    bool setDestDir, const char* tempInstallDirectory);
  virtual int InstallProjectViaInstallCMakeProjects(
    bool setDestDir, const char* tempInstallDirectory);

  virtual bool SupportsComponentInstallation() const;
  virtual cmCPackInstallationType* GetInstallationType(const char *projectName,
                                                       const char* name);
  virtual cmCPackComponent* GetComponent(const char *projectName,
                                         const char* name);
  virtual cmCPackComponentGroup* GetComponentGroup(const char *projectName,
                                                   const char* name);

  bool GeneratorVerbose;
  std::string Name;

  std::string InstallPath;

  std::string CPackSelf;
  std::string CMakeSelf;
  std::string CMakeRoot;

  std::map<std::string, cmCPackInstallationType> InstallationTypes;
  std::map<std::string, cmCPackComponent> Components;
  std::map<std::string, cmCPackComponentGroup> ComponentGroups;

  cmCPackLog* Logger;
private:
  cmMakefile* MakefileMap;
};

#endif
