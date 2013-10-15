/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2013 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef _cmRST_h
#define _cmRST_h

#include "cmStandardIncludes.h"

#include <cmsys/RegularExpression.hxx>

class cmRST
{
public:
  cmRST(std::ostream& os, std::string const& docroot);
  bool ProcessFile(std::string const& fname, bool isModule = false);
private:
  enum IncludeType
  {
    IncludeNormal,
    IncludeModule,
    IncludeTocTree
  };
  enum MarkupType
  {
    MarkupNone,
    MarkupNormal,
    MarkupEmpty
  };
  enum DirectiveType
  {
    DirectiveNone,
    DirectiveParsedLiteral,
    DirectiveCodeBlock,
    DirectiveReplace,
    DirectiveTocTree
  };

  void ProcessRST(std::istream& is);
  void ProcessModule(std::istream& is);
  void Reset();
  void ProcessLine(std::string const& line);
  void NormalLine(std::string const& line);
  void OutputLine(std::string const& line);
  std::string ReplaceSubstitutions(std::string const& line);
  bool ProcessInclude(std::string file, IncludeType type);
  void ProcessDirectiveParsedLiteral();
  void ProcessDirectiveCodeBlock();
  void ProcessDirectiveReplace();
  void ProcessDirectiveTocTree();
  static void UnindentLines(std::vector<std::string>& lines);

  std::ostream& OS;
  std::string DocRoot;
  int IncludeDepth;
  bool OutputLinePending;
  MarkupType Markup;
  DirectiveType Directive;
  cmsys::RegularExpression CMakeDirective;
  cmsys::RegularExpression CMakeModuleDirective;
  cmsys::RegularExpression ParsedLiteralDirective;
  cmsys::RegularExpression CodeBlockDirective;
  cmsys::RegularExpression ReplaceDirective;
  cmsys::RegularExpression IncludeDirective;
  cmsys::RegularExpression TocTreeDirective;
  cmsys::RegularExpression CMakeRole;
  cmsys::RegularExpression Substitution;

  std::vector<std::string> MarkupLines;
  std::string DocDir;
  std::map<cmStdString, cmStdString> Replace;
  std::set<cmStdString> Replaced;
  std::string ReplaceName;
};

#endif
