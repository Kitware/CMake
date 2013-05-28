/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGeneratorExpressionEvaluationFile_h
#define cmGeneratorExpressionEvaluationFile_h

#include "cmStandardIncludes.h"
#include <cmsys/auto_ptr.hxx>

#include "cmGeneratorExpression.h"

//----------------------------------------------------------------------------
class cmGeneratorExpressionEvaluationFile
{
public:
  cmGeneratorExpressionEvaluationFile(const std::string &input,
        cmsys::auto_ptr<cmCompiledGeneratorExpression> outputFileExpr,
        cmMakefile *makefile,
        cmsys::auto_ptr<cmCompiledGeneratorExpression> condition,
        bool inputIsContent);

  void Generate();

  std::vector<std::string> GetFiles() const { return this->Files; }

private:
  void Generate(const char *config,
              cmCompiledGeneratorExpression* inputExpression,
              std::map<std::string, std::string> &outputFiles);

private:
  const std::string Input;
  const cmsys::auto_ptr<cmCompiledGeneratorExpression> OutputFileExpr;
  cmMakefile *Makefile;
  const cmsys::auto_ptr<cmCompiledGeneratorExpression> Condition;
  std::vector<std::string> Files;
  const bool InputIsContent;
};

#endif
