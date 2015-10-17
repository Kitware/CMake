/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmExportTryCompileFileGenerator.h"

#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmGeneratorExpressionDAGChecker.h"

//----------------------------------------------------------------------------
cmExportTryCompileFileGenerator::cmExportTryCompileFileGenerator(
    cmGlobalGenerator* gg,
    const std::vector<std::string>& targets,
    cmMakefile* mf)
{
  gg->CreateImportedGenerationObjects(mf, targets, this->Exports);
}

bool cmExportTryCompileFileGenerator::GenerateMainFile(std::ostream& os)
{
  std::set<cmTarget const*> emitted;
  std::set<cmTarget const*> emittedDeps;
  while(!this->Exports.empty())
    {
    cmGeneratorTarget const* te = this->Exports.back();
    this->Exports.pop_back();
    if (emitted.insert(te->Target).second)
      {
      emittedDeps.insert(te->Target);
      this->GenerateImportTargetCode(os, te->Target);

      ImportPropertyMap properties;

#define FIND_TARGETS(PROPERTY) \
      this->FindTargets("INTERFACE_" #PROPERTY, te->Target, emittedDeps);

      CM_FOR_EACH_TRANSITIVE_PROPERTY_NAME(FIND_TARGETS)

#undef FIND_TARGETS

      this->PopulateProperties(te->Target, properties, emittedDeps);

      this->GenerateInterfaceProperties(te->Target, os, properties);
      }
    }
  return true;
}

std::string cmExportTryCompileFileGenerator::FindTargets(
                                          const std::string& propName,
                                          cmTarget const* tgt,
                                          std::set<cmTarget const*> &emitted)
{
  const char *prop = tgt->GetProperty(propName);
  if(!prop)
    {
    return std::string();
    }

  cmGeneratorExpression ge;

  cmGeneratorExpressionDAGChecker dagChecker(
                                      tgt->GetName(),
                                      propName, 0, 0);

  cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(prop);

  cmTarget dummyHead;
  dummyHead.SetType(cmState::EXECUTABLE, "try_compile_dummy_exe");
  dummyHead.SetMakefile(tgt->GetMakefile());

  cmGeneratorTarget* gtgt =
      tgt->GetMakefile()->GetGlobalGenerator()->GetGeneratorTarget(tgt);
  cmGeneratorTarget gDummyHead(&dummyHead, gtgt->GetLocalGenerator());

  std::string result = cge->Evaluate(gtgt->GetLocalGenerator(), this->Config,
                                     false, &gDummyHead,
                                     gtgt, &dagChecker);

  const std::set<cmGeneratorTarget const*> &allTargets =
      cge->GetAllTargetsSeen();
  for(std::set<cmGeneratorTarget const*>::const_iterator li =
      allTargets.begin(); li != allTargets.end(); ++li)
    {
    if(emitted.insert((*li)->Target).second)
      {
      this->Exports.push_back((*li));
      }
    }
  return result;
}

//----------------------------------------------------------------------------
void
cmExportTryCompileFileGenerator::PopulateProperties(cmTarget const* target,
                                                ImportPropertyMap& properties,
                                          std::set<cmTarget const*> &emitted)
{
  cmPropertyMap props = target->GetProperties();
  cmGeneratorTarget* gt =
      target->GetMakefile()->GetGlobalGenerator()->GetGeneratorTarget(target);
  for(cmPropertyMap::const_iterator i = props.begin(); i != props.end(); ++i)
    {
    properties[i->first] = i->second.GetValue();

    if(i->first.find("IMPORTED_LINK_INTERFACE_LIBRARIES") == 0
        || i->first.find("IMPORTED_LINK_DEPENDENT_LIBRARIES") == 0
        || i->first.find("INTERFACE_LINK_LIBRARIES") == 0)
      {
      std::string evalResult = this->FindTargets(i->first,
                                                 target, emitted);

      std::vector<std::string> depends;
      cmSystemTools::ExpandListArgument(evalResult, depends);
      for(std::vector<std::string>::const_iterator li = depends.begin();
          li != depends.end(); ++li)
        {
        cmGeneratorTarget *tgt =
            gt->GetLocalGenerator()->FindGeneratorTargetToUse(*li);
        if(tgt && emitted.insert(tgt->Target).second)
          {
          this->Exports.push_back(tgt);
          }
        }
      }
    }
}

std::string
cmExportTryCompileFileGenerator::InstallNameDir(cmGeneratorTarget* target,
                                                const std::string& config)
{
  std::string install_name_dir;

  cmMakefile* mf = target->Target->GetMakefile();
  if(mf->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME"))
    {
    install_name_dir =
      target->GetInstallNameDirForBuildTree(config);
    }

  return install_name_dir;
}
