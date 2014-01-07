/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmGeneratorExpressionEvaluationFile.h"

#include "cmMakefile.h"
#include <cmsys/FStream.hxx>

#include <assert.h>

//----------------------------------------------------------------------------
cmGeneratorExpressionEvaluationFile::cmGeneratorExpressionEvaluationFile(
        const std::string &input,
        cmsys::auto_ptr<cmCompiledGeneratorExpression> outputFileExpr,
        cmMakefile *makefile,
        cmsys::auto_ptr<cmCompiledGeneratorExpression> condition,
        bool inputIsContent)
  : Input(input),
    OutputFileExpr(outputFileExpr),
    Makefile(makefile),
    Condition(condition),
    InputIsContent(inputIsContent)
{
}

//----------------------------------------------------------------------------
void cmGeneratorExpressionEvaluationFile::Generate(const char *config,
              cmCompiledGeneratorExpression* inputExpression,
              std::map<std::string, std::string> &outputFiles)
{
  std::string rawCondition = this->Condition->GetInput();
  if (!rawCondition.empty())
    {
    std::string condResult = this->Condition->Evaluate(this->Makefile, config);
    if (condResult == "0")
      {
      return;
      }
    if (condResult != "1")
      {
      cmOStringStream e;
      e << "Evaluation file condition \"" << rawCondition << "\" did "
          "not evaluate to valid content. Got \"" << condResult << "\".";
      this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str().c_str());
      return;
      }
    }

  const std::string outputFileName
                    = this->OutputFileExpr->Evaluate(this->Makefile, config);
  const std::string outputContent
                          = inputExpression->Evaluate(this->Makefile, config);

  std::map<std::string, std::string>::iterator it
                                          = outputFiles.find(outputFileName);

  if(it != outputFiles.end())
    {
    if (it->second == outputContent)
      {
      return;
      }
    cmOStringStream e;
    e << "Evaluation file to be written multiple times for different "
         "configurations with different content:\n  " << outputFileName;
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str().c_str());
    return;
    }

  this->Files.push_back(outputFileName);
  outputFiles[outputFileName] = outputContent;

  cmsys::ofstream fout(outputFileName.c_str());

  if(!fout)
    {
    cmOStringStream e;
    e << "Evaluation file \"" << outputFileName << "\" cannot be written.";
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str().c_str());
    return;
    }

  fout << outputContent;

  fout.close();
}

//----------------------------------------------------------------------------
void cmGeneratorExpressionEvaluationFile::Generate()
{
  std::string inputContent;
  if (this->InputIsContent)
    {
    inputContent = this->Input;
    }
  else
    {
    cmsys::ifstream fin(this->Input.c_str());
    if(!fin)
      {
      cmOStringStream e;
      e << "Evaluation file \"" << this->Input << "\" cannot be read.";
      this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str().c_str());
      return;
      }

    std::string line;
    std::string sep;
    while(cmSystemTools::GetLineFromStream(fin, line))
      {
      inputContent += sep + line;
      sep = "\n";
      }
    inputContent += sep;
    }

  cmListFileBacktrace lfbt = this->OutputFileExpr->GetBacktrace();
  cmGeneratorExpression contentGE(lfbt);
  cmsys::auto_ptr<cmCompiledGeneratorExpression> inputExpression
                                              = contentGE.Parse(inputContent);

  std::map<std::string, std::string> outputFiles;

  std::vector<std::string> allConfigs;
  this->Makefile->GetConfigurations(allConfigs);

  if (allConfigs.empty())
    {
    this->Generate(0, inputExpression.get(), outputFiles);
    }
  else
    {
    for(std::vector<std::string>::const_iterator li = allConfigs.begin();
        li != allConfigs.end(); ++li)
      {
      this->Generate(li->c_str(), inputExpression.get(), outputFiles);
      if(cmSystemTools::GetFatalErrorOccured())
        {
        return;
        }
      }
    }
}
