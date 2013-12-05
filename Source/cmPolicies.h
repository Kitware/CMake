/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmPolicies_h
#define cmPolicies_h

#include "cmCustomCommand.h"

class cmake;
class cmMakefile;
class cmPolicy;

/** \class cmPolicies
 * \brief Handles changes in CMake behavior and policies
 *
 * See the cmake wiki section on
 * <a href="http://www.cmake.org/Wiki/CMake/Policies">policies</a>
 * for an overview of this class's purpose
 */
class cmPolicies
{
public:
  cmPolicies();
  ~cmPolicies();

  /// Status of a policy
  enum PolicyStatus {
    OLD, ///< Use old behavior
    WARN, ///< Use old behavior but issue a warning
    NEW, ///< Use new behavior
    /// Issue an error if user doesn't set policy status to NEW and hits the
    /// check
    REQUIRED_IF_USED,
    REQUIRED_ALWAYS ///< Issue an error unless user sets policy status to NEW.
  };
  static const char* PolicyStatusNames[];

  /// Policy identifiers
  enum PolicyID
  {
    CMP0000, ///< Policy version specification
    CMP0001, ///< Ignore old compatibility variable
    CMP0002, ///< Target names must be unique
    CMP0003, ///< Linking does not include extra -L paths
    CMP0004, ///< Libraries linked may not have leading or trailing whitespace
    CMP0005, ///< Definition value escaping
    CMP0006, ///< BUNDLE install rules needed for MACOSX_BUNDLE targets
    CMP0007, ///< list command handling of empty elements
    CMP0008, ///< Full-path libraries must be a valid library file name
    CMP0009, ///< GLOB_RECURSE should not follow symlinks by default
    CMP0010, ///< Bad variable reference syntax is an error
    CMP0011, ///< Strong policy scope for include and find_package
    CMP0012, ///< Recognize numbers and boolean constants in if()
    CMP0013, ///< Duplicate binary directories not allowed
    CMP0014, ///< Input directories must have CMakeLists.txt
    CMP0015, ///< link_directories() treats paths relative to source dir
    /// target_link_libraries() fails if only argument is not a target
    CMP0016,
    CMP0017, ///< Prefer files in CMAKE_ROOT when including from CMAKE_ROOT
    CMP0018, ///< Ignore language flags for shared libs, and adhere to
    /// POSITION_INDEPENDENT_CODE property and *_COMPILE_OPTIONS_PI{E,C}
    /// instead.
    CMP0019, ///< No variable re-expansion in include and link info
    CMP0020, ///< Automatically link Qt executables to qtmain target
    CMP0021, ///< Fatal error on relative paths in INCLUDE_DIRECTORIES
    /// target property
    CMP0022, ///< INTERFACE_LINK_LIBRARIES defines the link interface
    CMP0023, ///< Disallow mixing keyword and plain tll signatures
    CMP0024, ///< Disallow including export() result.
    CMP0025, ///< Compiler id for Apple Clang is now AppleClang
    CMP0026, ///< Disallow use of the LOCATION target property.
    CMP0027, ///< Conditionally linked imported targets with missing include
    /// directories.
    CMP0028, ///< Double colon in target name means ALIAS or IMPORTED target.
    CMP0029, ///< Disallow command: subdir_depends
    CMP0030, ///< Disallow command: use_mangled_mesa
    CMP0031, ///< Disallow command: load_command
    CMP0032, ///< Disallow command: output_required_files
    CMP0033, ///< Disallow command: export_library_dependencies
    CMP0034, ///< Disallow command: utility_source
    CMP0035, ///< Disallow command: variable_requires
    CMP0036, ///< Disallow command: build_name
    CMP0037, ///< Target names should not be reserved and
    /// should match a validity pattern.
    CMP0038, ///< Targets may not link directly to themselves
    CMP0039, ///< Utility targets may not have link dependencies
    CMP0040, ///< The target in the TARGET signature of
    /// add_custom_command() must exist.
    CMP0041, ///< Error on relative include with generator expression

    /** \brief Always the last entry.
     *
     * Useful mostly to avoid adding a comma the last policy when adding a new
     * one.
     */
    CMPCOUNT
  };

  ///! convert a string policy ID into a number
  bool GetPolicyID(const char *id, /* out */ cmPolicies::PolicyID &pid);
  std::string GetPolicyIDString(cmPolicies::PolicyID pid);

  ///! Get the default status for a policy
  cmPolicies::PolicyStatus GetPolicyStatus(cmPolicies::PolicyID id);

  ///! Define a Policy for CMake
  void DefinePolicy(cmPolicies::PolicyID id,
                    const char *stringID,
                    const char *shortDescription,
                    unsigned int majorVersionIntroduced,
                    unsigned int minorVersionIntroduced,
                    unsigned int patchVersionIntroduced,
                    unsigned int tweakVersionIntroduced,
                    cmPolicies::PolicyStatus status);

  ///! Set a policy level for this listfile
  bool ApplyPolicyVersion(cmMakefile *mf, const char *version);

  ///! return a warning string for a given policy
  std::string GetPolicyWarning(cmPolicies::PolicyID id);

  ///! return an error string for when a required policy is unspecified
  std::string GetRequiredPolicyError(cmPolicies::PolicyID id);

  ///! return an error string for when a required policy is unspecified
  std::string GetRequiredAlwaysPolicyError(cmPolicies::PolicyID id);

  /** Represent a set of policy values.  */
  typedef std::map<PolicyID, PolicyStatus> PolicyMap;

  private:
  // might have to make these internal for VS6 not sure yet
  std::map<PolicyID,cmPolicy *> Policies;
  std::map<std::string,PolicyID> PolicyStringMap;

  void DiagnoseAncientPolicies(std::vector<PolicyID> const& ancient,
                               unsigned int majorVer, unsigned int minorVer,
                               unsigned int patchVer, cmMakefile* mf);

  bool GetPolicyDefault(cmMakefile* mf, std::string const& policy,
                        cmPolicies::PolicyStatus* defaultStatus);

};

#endif
