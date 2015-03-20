#ifndef CMEXTRAQBSGENERATOR_H
#define CMEXTRAQBSGENERATOR_H

#include "cmExternalMakefileProjectGenerator.h"

class cmGeneratorTarget;

class cmExtraQbsGenerator : public cmExternalMakefileProjectGenerator
{
public:
  cmExtraQbsGenerator();
  ~cmExtraQbsGenerator();

  virtual std::string GetName() const
  { return cmExtraQbsGenerator::GetActualName(); }
  static std::string GetActualName() { return "Qbs"; }
  static cmExternalMakefileProjectGenerator *New()
  { return new cmExtraQbsGenerator; }

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry &entry,
                                const std::string &fullName) const;

  virtual void Generate();

private:
  void CreateProjectFile(const std::string &name,
                         const std::vector<cmLocalGenerator *> &lgs);
  void CreateNewProjectFile(const std::string &projectName,
                            const std::vector<cmLocalGenerator *> &lgs,
                            const std::string &filename);
  void AppendSubProject(cmGeneratedFileStream &fout, cmLocalGenerator *lg);
  void AppendProduct(cmGeneratedFileStream &fout, cmLocalGenerator *lg);
  void AppendTarget(cmGeneratedFileStream &fout,
                    cmLocalGenerator *lg,
                    const cmTarget &t,
                    const std::string &cfg);
  void AppendSources(cmGeneratedFileStream &fout,
                     cmGeneratorTarget *gt,
                     const cmTarget &t,
                     const std::string &cfg);
  void AppendIncludePaths(cmGeneratedFileStream &fout,
                          const std::set<std::string> &paths);
  void AppendCompileDefinitions(cmGeneratedFileStream &fout,
                                const std::set<std::string> &defs);
};

#endif // CMEXTRAQBSGENERATOR_H
