/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmStandardIncludes.h"

#include <stack>

#include <cmsys/RegularExpression.hxx>

class cmMakefile;
class cmListFileBacktrace;

/** \class cmGeneratorExpression
 * \brief Evaluate generate-time query expression syntax.
 *
 * cmGeneratorExpression instances are used by build system generator
 * implementations to evaluate the $<> generator expression syntax.
 * Generator expressions are evaluated just before the generate step
 * writes strings into the build system.  They have knowledge of the
 * build configuration which is not available at configure time.
 */
class cmGeneratorExpression
{
public:
  /** Construct with an evaluation context and configuration.  */
  cmGeneratorExpression(cmMakefile* mf, const char* config,
                        cmListFileBacktrace const& backtrace);

  /** Evaluate generator expressions in a string.  */
  const char* Process(std::string const& input);
  const char* Process(const char* input);
private:
  cmMakefile* Makefile;
  const char* Config;
  cmListFileBacktrace const& Backtrace;
  std::vector<char> Data;
  std::stack<size_t> Barriers;
  cmsys::RegularExpression TargetInfo;
  bool Evaluate();
  bool Evaluate(const char* expr, std::string& result);
  bool EvaluateTargetInfo(std::string& result);
};
