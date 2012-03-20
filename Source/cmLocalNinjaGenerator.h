/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2011 Peter Collingbourne <peter@pcc.me.uk>
  Copyright 2011 Nicolas Despres <nicolas.despres@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmLocalNinjaGenerator_h
#  define cmLocalNinjaGenerator_h

#  include "cmLocalGenerator.h"
#  include "cmNinjaTypes.h"

class cmGlobalNinjaGenerator;
class cmGeneratedFileStream;
class cmake;

/**
 * \class cmLocalNinjaGenerator
 * \brief Write a local build.ninja file.
 *
 * cmLocalNinjaGenerator produces a local build.ninja file from its
 * member Makefile.
 */
class cmLocalNinjaGenerator : public cmLocalGenerator
{
public:
  /// Default constructor.
  cmLocalNinjaGenerator();

  /// Destructor.
  virtual ~cmLocalNinjaGenerator();

  /// Overloaded methods. @see cmLocalGenerator::Generate()
  virtual void Generate();

  /// Overloaded methods. @see cmLocalGenerator::Configure()
  virtual void Configure();

  /// Overloaded methods. @see cmLocalGenerator::GetTargetDirectory()
  virtual std::string GetTargetDirectory(cmTarget const& target) const;

public:
  const cmGlobalNinjaGenerator* GetGlobalNinjaGenerator() const;
  cmGlobalNinjaGenerator* GetGlobalNinjaGenerator();

  /**
   * Shortcut to get the cmake instance throw the global generator.
   * @return an instance of the cmake object.
   */
  const cmake* GetCMakeInstance() const;
  cmake* GetCMakeInstance();

  const char* GetConfigName() const
  { return this->ConfigName.c_str(); }

  /// @return whether we are processing the top CMakeLists.txt file.
  bool isRootMakefile() const;

  /// @returns the relative path between the HomeOutputDirectory and this
  /// local generators StartOutputDirectory.
  std::string GetHomeRelativeOutputPath() const
  { return this->HomeRelativeOutputPath; }

protected:
  virtual std::string ConvertToLinkReference(std::string const& lib);
  virtual std::string ConvertToIncludeReference(std::string const& path);

private:
  friend class cmGlobalNinjaGenerator;

  // In order to access to protected member of the local generator.
  friend class cmNinjaTargetGenerator;
  friend class cmNinjaNormalTargetGenerator;
  friend class cmNinjaUtilityTargetGenerator;

private:
  cmGeneratedFileStream& GetBuildFileStream() const;
  cmGeneratedFileStream& GetRulesFileStream() const;

  void WriteBuildFileTop();
  void WriteProjectHeader(std::ostream& os);
  void WriteNinjaFilesInclusion(std::ostream& os);
  void WriteProcessedMakefile(std::ostream& os);

  void SetConfigName();

  std::string ConvertToNinjaPath(const char *path);

  struct map_to_ninja_path;
  friend struct map_to_ninja_path;
  struct map_to_ninja_path {
    cmLocalNinjaGenerator *LocalGenerator;
    map_to_ninja_path(cmLocalNinjaGenerator *LocalGen)
      : LocalGenerator(LocalGen) {}
    std::string operator()(const std::string &path) {
      return LocalGenerator->ConvertToNinjaPath(path.c_str());
    }
  };
  map_to_ninja_path MapToNinjaPath() {
    return map_to_ninja_path(this);
  }

  void AppendTargetOutputs(cmTarget* target, cmNinjaDeps& outputs);
  void AppendTargetDepends(cmTarget* target, cmNinjaDeps& outputs);

  void AppendCustomCommandDeps(const cmCustomCommand *cc,
                               cmNinjaDeps &ninjaDeps);
  std::string BuildCommandLine(const std::vector<std::string> &cmdLines);
  void AppendCustomCommandLines(const cmCustomCommand *cc,
                                std::vector<std::string> &cmdLines);
  void WriteCustomCommandRule();
  void WriteCustomCommandBuildStatement(cmCustomCommand const *cc,
                                        const cmNinjaDeps& orderOnlyDeps);

  void AddCustomCommandTarget(cmCustomCommand const* cc, cmTarget* target);
  void WriteCustomCommandBuildStatements();

private:
  std::string ConfigName;
  std::string HomeRelativeOutputPath;

  typedef std::map<cmCustomCommand const*, std::set<cmTarget*> >
    CustomCommandTargetMap;
  CustomCommandTargetMap CustomCommandTargets;
};

#endif // ! cmLocalNinjaGenerator_h
