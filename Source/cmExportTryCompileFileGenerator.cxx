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
#include "cmGeneratorExpressionDAGChecker.h"

//----------------------------------------------------------------------------
bool cmExportTryCompileFileGenerator::GenerateMainFile(std::ostream& os)
{
  std::set<cmTarget*> emitted;
  std::set<cmTarget*> emittedDeps;
  while(!this->Exports.empty())
    {
    cmTarget* te = this->Exports.back();
    this->Exports.pop_back();
    if (emitted.insert(te).second)
      {
      emittedDeps.insert(te);
      this->GenerateImportTargetCode(os, te);

      ImportPropertyMap properties;

      this->FindTargets("INTERFACE_INCLUDE_DIRECTORIES", te, emittedDeps);
      this->FindTargets("INTERFACE_COMPILE_DEFINITIONS", te, emittedDeps);

      this->PopulateProperties(te, properties, emittedDeps);

      this->GenerateInterfaceProperties(te, os, properties);
      }
    }
  return true;
}

std::string cmExportTryCompileFileGenerator::FindTargets(const char *propName,
                                                cmTarget *tgt,
                                                std::set<cmTarget*> &emitted)
{
  const char *prop = tgt->GetProperty(propName);
  if(!prop)
    {
    return std::string();
    }

  cmListFileBacktrace lfbt;
  cmGeneratorExpression ge(lfbt);

  cmGeneratorExpressionDAGChecker dagChecker(lfbt,
                                      tgt->GetName(),
                                      propName, 0, 0);

  cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(prop);

  cmTarget dummyHead;
  dummyHead.SetType(cmTarget::EXECUTABLE, "try_compile_dummy_exe");
  dummyHead.SetMakefile(tgt->GetMakefile());

  std::string result = cge->Evaluate(tgt->GetMakefile(), this->Config,
                                     false, &dummyHead, tgt, &dagChecker);

  const std::set<cmTarget*> &allTargets = cge->GetAllTargetsSeen();
  for(std::set<cmTarget*>::const_iterator li = allTargets.begin();
      li != allTargets.end(); ++li)
    {
    if(emitted.insert(*li).second)
      {
      this->Exports.push_back(*li);
      }
    }
  return result;
}

//----------------------------------------------------------------------------
void
cmExportTryCompileFileGenerator::PopulateProperties(cmTarget* target,
                                                ImportPropertyMap& properties,
                                                std::set<cmTarget*> &emitted)
{
  cmPropertyMap props = target->GetProperties();
  for(cmPropertyMap::const_iterator i = props.begin(); i != props.end(); ++i)
    {
    properties[i->first] = i->second.GetValue();

    if(i->first.find("IMPORTED_LINK_INTERFACE_LIBRARIES") == 0)
      {
      const std::string libs = i->second.GetValue();

      std::string evalResult = this->FindTargets(i->first.c_str(),
                                                 target, emitted);

      std::vector<std::string> depends;
      cmSystemTools::ExpandListArgument(evalResult, depends);
      for(std::vector<std::string>::const_iterator li = depends.begin();
          li != depends.end(); ++li)
        {
        cmTarget *tgt = target->GetMakefile()->FindTargetToUse(li->c_str());
        if(tgt && emitted.insert(tgt).second)
          {
          this->Exports.push_back(tgt);
          }
        }
      }
    }
}
