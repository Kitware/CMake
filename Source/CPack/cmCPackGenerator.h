/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <cm/optional>
#include <cm/string_view>

#include "cm_sys_stat.h"

#include "cmCPackComponentGroup.h"
#include "cmSystemTools.h"
#include "cmValue.h"

class cmCPackLog;
class cmCryptoHash;
class cmGlobalGenerator;
class cmInstalledFile;
class cmMakefile;

/** \class cmCPackGenerator
 * \brief A superclass of all CPack Generators
 *
 */
class cmCPackGenerator
{
public:
  virtual char const* GetNameOfClass() = 0;
  /**
   * If verbose then more information is printed out
   */
  void SetVerbose(bool val)
  {
    this->GeneratorVerbose =
      val ? cmSystemTools::OUTPUT_MERGE : cmSystemTools::OUTPUT_NONE;
  }

  /**
   * Put underlying cmake scripts in trace mode.
   */
  void SetTrace(bool val) { this->Trace = val; }

  /**
   * Put underlying cmake scripts in expanded trace mode.
   */
  void SetTraceExpand(bool val) { this->TraceExpand = val; }

  /**
   * Returns true if the generator may work on this system.
   * Rational:
   * Some CPack generator may run on some host and may not on others
   * (with the same system) because some tools are missing. If the tool
   * is missing then CPack won't activate (in the CPackGeneratorFactory)
   * this particular generator.
   */
  static bool CanGenerate() { return true; }

  /**
   * Do the actual whole package processing.
   * Subclass may redefine it but its usually enough
   * to redefine @ref PackageFiles, because in fact
   * this method do call:
   *     - PrepareName
   *     - clean-up temp dirs
   *     - InstallProject (with the appropriate method)
   *     - prepare list of files and/or components to be package
   *     - PackageFiles
   *     - Copy produced packages at the expected place
   * @return 0 if error.
   */
  virtual int DoPackage();

  /**
   * Initialize generator
   */
  int Initialize(std::string const& name, cmMakefile* mf);

  /**
   * Construct generator
   */
  cmCPackGenerator();
  virtual ~cmCPackGenerator();

  //! Set and get the options
  void SetOption(std::string const& op, char const* value);
  void SetOption(std::string const& op, std::string const& value)
  {
    this->SetOption(op, cmValue(value));
  }
  void SetOption(std::string const& op, cmValue value);
  void SetOptionIfNotSet(std::string const& op, char const* value);
  void SetOptionIfNotSet(std::string const& op, std::string const& value)
  {
    this->SetOptionIfNotSet(op, cmValue(value));
  }
  void SetOptionIfNotSet(std::string const& op, cmValue value);
  cmValue GetOption(std::string const& op) const;
  std::vector<std::string> GetOptions() const;
  bool IsSet(std::string const& name) const;
  cmValue GetOptionIfSet(std::string const& name) const;
  bool IsOn(std::string const& name) const;
  bool IsSetToOff(std::string const& op) const;
  bool IsSetToEmpty(std::string const& op) const;

  //! Set the logger
  void SetLogger(cmCPackLog* log) { this->Logger = log; }

  //! Display verbose information via logger
  void DisplayVerboseOutput(std::string const& msg, float progress);

  bool ReadListFile(char const* moduleName);

protected:
  /**
   * Prepare common used names by inspecting
   * several CPACK_xxx var values.
   */
  int PrepareNames();

  /**
   * Install the project using appropriate method.
   */
  int InstallProject();

  int CleanTemporaryDirectory();

  cmInstalledFile const* GetInstalledFile(std::string const& name) const;

  virtual char const* GetOutputExtension() { return ".cpack"; }
  virtual char const* GetOutputPostfix() { return nullptr; }

  /**
   * Prepare requested grouping kind from CPACK_xxx vars
   * CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE
   * CPACK_COMPONENTS_IGNORE_GROUPS
   * or
   * CPACK_COMPONENTS_ONE_PACKAGE_PER_GROUP
   * @return 1 on success 0 on failure.
   */
  virtual int PrepareGroupingKind();

  /**
   * Ensures that the given name only contains characters that can cleanly be
   * used as directory or file name and returns this sanitized name. Possibly,
   * this name might be replaced by its hash.
   * @param[in] name the name for a directory or file that shall be sanitized.
   * @param[in] isFullName true if the result is used as the full name for a
   *            directory or file. (Defaults to true.)
   * @return the sanitized name.
   */
  virtual std::string GetSanitizedDirOrFileName(std::string const& name,
                                                bool isFullName = true) const;

  /**
   * Some CPack generators may prefer to have
   * CPack install all components belonging to the same
   * [component] group to be install in the same directory.
   * The default behavior is to install each component in
   * a separate directory.
   * @param[in] componentName the name of the component to be installed
   * @return the name suffix the generator wants for the specified component
   *         default is "componentName"
   */
  virtual std::string GetComponentInstallSuffix(
    std::string const& componentName);

  /**
   * The value that GetComponentInstallSuffix returns, but sanitized.
   * @param[in] componentName the name of the component to be installed
   * @return the name suffix the generator wants for the specified component
   *         (but sanitized, so that it can be used on the file-system).
   *         default is "componentName".
   */
  virtual std::string GetComponentInstallDirNameSuffix(
    std::string const& componentName);

  /**
   * CPack specific generator may mangle CPACK_PACKAGE_FILE_NAME
   * with CPACK_COMPONENT_xxxx_<NAME>_DISPLAY_NAME if
   * CPACK_<GEN>_USE_DISPLAY_NAME_IN_FILENAME is ON.
   * @param[in] initialPackageFileName the initial package name to be mangled
   * @param[in] groupOrComponentName the name of the group/component
   * @param[in] isGroupName true if previous name refers to a group,
   *            false otherwise
   */
  virtual std::string GetComponentPackageFileName(
    std::string const& initialPackageFileName,
    std::string const& groupOrComponentName, bool isGroupName);

  /**
   * Package the list of files and/or components which
   * has been prepared by the beginning of DoPackage.
   * @pre the @ref toplevel has been filled-in
   * @pre the list of file @ref files has been populated
   * @pre packageFileNames contains at least 1 entry
   * @post packageFileNames may have been updated and contains
   *       the list of packages generated by the specific generator.
   */
  virtual int PackageFiles();
  virtual char const* GetInstallPath();
  virtual char const* GetPackagingInstallPrefix();

  bool GenerateChecksumFile(cmCryptoHash& crypto,
                            cm::string_view filename) const;
  bool CopyPackageFile(std::string const& srcFilePath,
                       cm::string_view filename) const;

  std::string FindTemplate(cm::string_view name,
                           cm::optional<cm::string_view> alt = cm::nullopt);
  virtual bool ConfigureFile(std::string const& inName,
                             std::string const& outName,
                             bool copyOnly = false);
  virtual bool ConfigureString(std::string const& input, std::string& output);
  virtual int InitializeInternal();

  //! Run install commands if specified
  virtual int InstallProjectViaInstallCommands(
    bool setDestDir, std::string const& tempInstallDirectory);
  virtual int InstallProjectViaInstallScript(
    bool setDestDir, std::string const& tempInstallDirectory);
  virtual int InstallProjectViaInstalledDirectories(
    bool setDestDir, std::string const& tempInstallDirectory,
    mode_t const* default_dir_mode);
  virtual int InstallProjectViaInstallCMakeProjects(
    bool setDestDir, std::string const& tempInstallDirectory,
    mode_t const* default_dir_mode);

  virtual int RunPreinstallTarget(std::string const& installProjectName,
                                  std::string const& installDirectory,
                                  cmGlobalGenerator* globalGenerator,
                                  std::string const& buildConfig);
  virtual int InstallCMakeProject(
    bool setDestDir, std::string const& installDirectory,
    std::string const& baseTempInstallDirectory,
    mode_t const* default_dir_mode, std::string const& component,
    bool componentInstall, std::string const& installSubDirectory,
    std::string const& buildConfig, std::string& absoluteDestFiles);

  /**
   * The various level of support of
   * CPACK_SET_DESTDIR used by the generator.
   */
  enum CPackSetDestdirSupport
  {
    /* the generator works with or without it */
    SETDESTDIR_SUPPORTED,
    /* the generator works best if automatically handled */
    SETDESTDIR_INTERNALLY_SUPPORTED,
    /* no official support, use at your own risk */
    SETDESTDIR_SHOULD_NOT_BE_USED,
    /* officially NOT supported */
    SETDESTDIR_UNSUPPORTED
  };

  /**
   * Does the CPack generator support CPACK_SET_DESTDIR?
   * The default legacy value is 'SETDESTDIR_SUPPORTED' generator
   * have to override it in order change this.
   * @return CPackSetDestdirSupport
   */
  virtual enum CPackSetDestdirSupport SupportsSetDestdir() const;

  /**
   * Does the CPack generator support absolute path
   * in INSTALL DESTINATION?
   * The default legacy value is 'true' generator
   * have to override it in order change this.
   * @return true if supported false otherwise
   */
  virtual bool SupportsAbsoluteDestination() const;

  /**
   * Does the CPack generator support component installation?.
   * Some Generators requires the user to set
   * CPACK_<GENNAME>_COMPONENT_INSTALL in order to make this
   * method return true.
   * @return true if supported, false otherwise
   */
  virtual bool SupportsComponentInstallation() const;
  /**
   * Does the currently running generator want a component installation.
   * The generator may support component installation but he may
   * be requiring monolithic install using CPACK_MONOLITHIC_INSTALL.
   * @return true if component installation is supported and wanted.
   */
  virtual bool WantsComponentInstallation() const;
  virtual cmCPackInstallationType* GetInstallationType(
    std::string const& projectName, std::string const& name);
  virtual cmCPackComponent* GetComponent(std::string const& projectName,
                                         std::string const& name);
  virtual cmCPackComponentGroup* GetComponentGroup(
    std::string const& projectName, std::string const& name);

  cmSystemTools::OutputOption GeneratorVerbose;
  std::string Name;

  std::string InstallPath;

  /**
   * The list of package file names.
   * At beginning of DoPackage the (generic) generator will populate
   * the list of desired package file names then it will
   * call the redefined method PackageFiles which is may
   * either use this set of names (usually on entry there should be
   * only a single name) or update the vector with the list
   * of created package file names.
   */
  std::vector<std::string> packageFileNames;

  /**
   * The directory where all the files to be packaged reside.
   * If the installer support components there will be one
   * sub-directory for each component. In those directories
   * one will find the file belonging to the specified component.
   */
  std::string toplevel;

  /**
   * The complete list of files to be packaged.
   * This list will be populated by DoPackage before
   * PackageFiles is called.
   */
  std::vector<std::string> files;

  std::vector<cmCPackInstallCMakeProject> CMakeProjects;
  std::map<std::string, cmCPackInstallationType> InstallationTypes;
  /**
   * The set of components.
   * If component installation is supported then this map
   * contains the component specified in CPACK_COMPONENTS_ALL
   */
  std::map<std::string, cmCPackComponent> Components;
  std::map<std::string, cmCPackComponentGroup> ComponentGroups;

  /**
   * If components are enabled, this enum represents the different
   * ways of mapping components to package files.
   */
  enum ComponentPackageMethod
  {
    /* one package for all components */
    ONE_PACKAGE,
    /* one package for each component */
    ONE_PACKAGE_PER_COMPONENT,
    /* one package for each group,
     * with left over components in their own package */
    ONE_PACKAGE_PER_GROUP,
    UNKNOWN_COMPONENT_PACKAGE_METHOD
  };

  /**
   * The component package method
   * The default is ONE_PACKAGE_PER_GROUP,
   * and generators may override the default
   * before PrepareGroupingKind() is called.
   */
  ComponentPackageMethod componentPackageMethod;

  cmCPackLog* Logger;
  bool Trace;
  bool TraceExpand;

  cmMakefile* MakefileMap;

private:
  template <typename ValueType>
  void StoreOption(std::string const& op, ValueType value);
  template <typename ValueType>
  void StoreOptionIfNotSet(std::string const& op, ValueType value);
};

#define cmCPackTypeMacro(klass, superclass)                                   \
  using Superclass = superclass;                                              \
  const char* GetNameOfClass() override                                       \
  {                                                                           \
    return #klass;                                                            \
  }                                                                           \
  static cmCPackGenerator* CreateGenerator()                                  \
  {                                                                           \
    return new klass;                                                         \
  }                                                                           \
  class cmCPackTypeMacro_UseTrailingSemicolon

#define cmCPackLogger(logType, msg)                                           \
  do {                                                                        \
    std::ostringstream cmCPackLog_msg;                                        \
    cmCPackLog_msg << msg;                                                    \
    this->Logger->Log(logType, __FILE__, __LINE__,                            \
                      cmCPackLog_msg.str().c_str());                          \
  } while (false)
