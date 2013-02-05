/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2012 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGeneratorTarget_h
#define cmGeneratorTarget_h

#include "cmStandardIncludes.h"

class cmCustomCommand;
class cmGlobalGenerator;
class cmLocalGenerator;
class cmMakefile;
class cmSourceFile;
class cmTarget;

class cmGeneratorTarget
{
public:
  cmGeneratorTarget(cmTarget*);

  int GetType() const;
  const char *GetName() const;
  const char *GetProperty(const char *prop);
  bool GetPropertyAsBool(const char *prop);
  std::vector<cmSourceFile*> const& GetSourceFiles();

  cmTarget* Target;
  cmMakefile* Makefile;
  cmLocalGenerator* LocalGenerator;
  cmGlobalGenerator* GlobalGenerator;

  /** Sources classified by purpose.  */
  std::vector<cmSourceFile*> CustomCommands;
  std::vector<cmSourceFile*> ExtraSources;
  std::vector<cmSourceFile*> HeaderSources;
  std::vector<cmSourceFile*> ObjectSources;
  std::vector<cmSourceFile*> ExternalObjects;
  std::vector<cmSourceFile*> IDLSources;
  std::string ModuleDefinitionFile;

  std::map<cmSourceFile const*, std::string> Objects;
  std::set<cmSourceFile const*> ExplicitObjectName;

  /** Full path with trailing slash to the top-level directory
      holding object files for this target.  Includes the build
      time config name placeholder if needed for the generator.  */
  std::string ObjectDirectory;

  std::vector<cmTarget*> ObjectLibraries;

  void UseObjectLibraries(std::vector<std::string>& objs);

  void GetAppleArchs(const char* config,
                     std::vector<std::string>& archVec);

  ///! Return the rule variable used to create this type of target,
  //  need to add CMAKE_(LANG) for full name.
  const char* GetCreateRuleVariable();

  /** Get the include directories for this target.  */
  std::vector<std::string> GetIncludeDirectories(const char *config);

private:
  void ClassifySources();
  void LookupObjectLibraries();

  cmGeneratorTarget(cmGeneratorTarget const&);
  void operator=(cmGeneratorTarget const&);
};

typedef std::map<cmTarget*, cmGeneratorTarget*> cmGeneratorTargetsType;

#endif
