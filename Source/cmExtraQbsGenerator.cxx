#include "cmExtraQbsGenerator.h"

#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmSourceFile.h"

cmExtraQbsGenerator::cmExtraQbsGenerator()
{
#if defined(_WIN32)
  this->SupportedGlobalGenerators.push_back("MinGW Makefiles");
  this->SupportedGlobalGenerators.push_back("NMake Makefiles");
#endif
  this->SupportedGlobalGenerators.push_back("Ninja");
  this->SupportedGlobalGenerators.push_back("Unix Makefiles");
}

cmExtraQbsGenerator::~cmExtraQbsGenerator() {}

void cmExtraQbsGenerator::GetDocumentation(cmDocumentationEntry &entry,
                                           const std::string &) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates Qbs project files.";
}

void cmExtraQbsGenerator::Generate()
{
  for (std::map<std::string, std::vector<cmLocalGenerator *> >::const_iterator
       it = this->GlobalGenerator->GetProjectMap().begin();
       it != this->GlobalGenerator->GetProjectMap().end(); ++it)
    {
    // create a project file
    this->CreateProjectFile(it->first, it->second);
    }
}

void cmExtraQbsGenerator::CreateProjectFile(
        const std::string &name,
        const std::vector<cmLocalGenerator *> &lgs)
{
  const cmMakefile *mf = lgs[0]->GetMakefile();
  std::string outputDir = mf->GetCurrentBinaryDirectory();

  const std::string filename = outputDir + "/" + name + ".qbs";

  this->CreateNewProjectFile(name, lgs, filename);
}

void cmExtraQbsGenerator::CreateNewProjectFile(
    const std::string &projectName, const std::vector<cmLocalGenerator *> &lgs,
    const std::string &filename)
{
  cmGeneratedFileStream fout(filename.c_str());
  if (!fout)
    {
    return;
    }

  fout << "import qbs\n"
       << "import qbs.File\n\n"
       << "Project {\n"
       << "\tname:\"" << projectName << "\"\n";
  std::vector<cmLocalGenerator *>::const_iterator itr = lgs.begin();
  for (; itr != lgs.end(); ++itr)
    {
    cmLocalGenerator *lg = (*itr);
    this->AppendSubProject(fout, lg);
    }
  fout << "}\n";
}

void cmExtraQbsGenerator::AppendSubProject(cmGeneratedFileStream &fout,
                                           cmLocalGenerator *lg)
{
  const cmMakefile *mk = lg->GetMakefile();
  if (!mk || mk->GetTargets().size() == 0)
    {
    return;
    }

  const std::string &relativePath = cmSystemTools::RelativePath(
      mk->GetHomeDirectory(), mk->GetCurrentSourceDirectory());
  fout << "\tProject {\n"
       << "\t\tname:\"" << relativePath << "\"\n";
  this->AppendProduct(fout, lg);
  fout << "\t}\n";
}

void cmExtraQbsGenerator::AppendProduct(cmGeneratedFileStream &fout,
                                        cmLocalGenerator *lg)
{
  const cmMakefile *mk = lg->GetMakefile();
  const cmTargets &ts = mk->GetTargets();
  std::string cfg = mk->GetSafeDefinition("CMAKE_BUILD_TYPE");
  cmTargets::const_iterator itr = ts.begin();
  for (; itr != ts.end(); ++itr)
    {
    const cmTarget &t = itr->second;
    this->AppendTarget(fout, lg, t, cfg);
    }
}

void cmExtraQbsGenerator::AppendTarget(cmGeneratedFileStream &fout,
                                       cmLocalGenerator *lg, const cmTarget &t,
                                       const std::string &cfg)
{
  std::string type;
  bool isBuildable = true;
  switch (t.GetType())
    {
    case cmTarget::EXECUTABLE:
      type = "application";
      break;
    case cmTarget::SHARED_LIBRARY:
      type = "dynamiclibrary";
      break;
    case cmTarget::STATIC_LIBRARY:
      type = "staticlibrary";
      break;
    default:
      isBuildable = false;
      break;
    }

  if (type.empty())
    {
    fout << "\t\tProject {\n";
    }
  else
    {
    fout << "\t\tProduct {\n";
    fout << "\t\t\tdestinationDirectory: \"" << t.GetDirectory(cfg) << "\"\n";
    }
  fout << "\t\t\tname:\"" << t.GetName() << "\"\n";

  if (!type.empty())
    {
    fout << "\t\t\ttype: \"" << type << "\"\n";
    fout << "\t\t\ttargetName: \"" << t.GetName() << "\"\n";
    }

  if (isBuildable)
    {
    fout << "\t\t\tDepends { name: \"cpp\" }\n";
    cmGeneratorTarget *gt = this->GlobalGenerator->GetGeneratorTarget(&t);
    this->AppendSources(fout, gt, t, cfg);

    std::set<std::string> langs, incPaths, defs;
    t.GetLanguages(langs, cfg);
    for (std::set<std::string>::const_iterator lang = langs.begin();
         lang != langs.end();
         ++ lang)
      {
      const std::vector<std::string> &paths =
          gt->GetIncludeDirectories(cfg, *lang);
      std::copy(paths.begin(), paths.end(),
                std::inserter(incPaths, incPaths.end()));

      lg->AddCompileDefinitions(defs, &t, cfg, *lang);
      }
    this->AppendIncludePaths(fout, incPaths);
    this->AppendCompileDefinitions(fout, defs);
    }

  fout << "\t\t}\n";
}

void cmExtraQbsGenerator::AppendSources(cmGeneratedFileStream &fout,
                                        cmGeneratorTarget *gt,
                                        const cmTarget &t,
                                        const std::string &cfg)
{
  std::vector<cmSourceFile *> sources;
  gt->GetSourceFiles(sources, cfg);
  if (sources.empty())
    {
    return;
    }

  std::vector<cmSourceFile *> genSources;
  std::vector<cmSourceFile *>::const_iterator itr = sources.begin();
  fout << "\t\t\tfiles: [\n"
       << "\t\t\t\t\""
       << t.GetMakefile()->GetDefinition("CMAKE_CURRENT_LIST_FILE")
       << "\",\n";
  for (; itr != sources.end(); ++itr)
    {
    if (!(*itr)->GetPropertyAsBool("GENERATED"))
      {
      fout << "\t\t\t\t\"" << (*itr)->GetFullPath() << "\",\n";
      }
    else
      {
      genSources.push_back(*itr);
      }
    }
  fout << "\t\t\t]\n";

  if (!genSources.empty())
    {
    fout << "\t\t\tGroup {\n"
         << "\t\t\t\tname:\"Generated\"\n"
         << "\t\t\t\tfiles: [\n";
    itr = genSources.begin();
    std::string groupCondition;
    bool initialCondition = true;
    for (; itr != genSources.end(); ++itr)
      {
      const std::string &path = (*itr)->GetFullPath();
      fout << "\t\t\t\t\t\"" << path << "\",\n";
      if (initialCondition)
        {
        initialCondition = false;
        }
      else
        {
        groupCondition += "\t\t\t\t\t && ";
        }
      groupCondition += "File.exists(\"" + path + "\")\n";
      }
    fout << "\t\t\t\t]\n"
         << "\t\t\t\tcondition: " << groupCondition << "\t\t\t}\n";
  }
}

void cmExtraQbsGenerator::AppendIncludePaths(
    cmGeneratedFileStream &fout,
    const std::set<std::string> &paths)
{
  if (paths.empty())
    {
    return;
    }

  std::set<std::string>::const_iterator itr = paths.begin();
  fout << "\t\t\tcpp.includePaths: [\n";
  for (; itr != paths.end(); ++ itr)
    {
    fout << "\t\t\t\t\"" << (*itr) << "\",\n";
    }
  fout << "\t\t\t]\n";
}

void cmExtraQbsGenerator::AppendCompileDefinitions(
    cmGeneratedFileStream &fout,
    const std::set<std::string> &defs)
{
  if (defs.empty())
    {
    return;
    }

  std::set<std::string>::const_iterator itr = defs.begin();
  fout << "\t\t\tcpp.defines: [\n";
  for (; itr != defs.end(); ++ itr)
    {
    fout << "\t\t\t\t'" << (*itr) << "',\n";
    }
  fout << "\t\t\t]\n";
}
