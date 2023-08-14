/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "cmsys/RegularExpression.hxx"

/** \class cmRST
 * \brief Perform basic .rst processing for command-line help
 *
 * This class implements a subset of reStructuredText and Sphinx
 * document processing.  It is used to print command-line help.
 *
 * If you modify the capabilities of this class, be sure to update
 * the Help/manual/cmake-developer.7.rst documentation and to update
 * the Tests/CMakeLib/testRST.(rst|expect) test input and output.
 */
class cmRST
{
public:
  cmRST(std::ostream& os, std::string docroot);
  bool ProcessFile(std::string const& fname, bool isModule = false);

private:
  enum class Include
  {
    Normal,
    Module,
    TocTree
  };
  enum class Markup
  {
    None,
    Normal,
    Empty
  };
  enum class Directive
  {
    None,
    ParsedLiteral,
    LiteralBlock,
    CodeBlock,
    Replace,
    TocTree
  };

  void ProcessRST(std::istream& is);
  void ProcessModule(std::istream& is);
  void Reset();
  void ProcessLine(std::string const& line);
  void NormalLine(std::string const& line);
  void OutputLine(std::string const& line, bool inlineMarkup);
  std::string ReplaceSubstitutions(std::string const& line);
  void OutputMarkupLines(bool inlineMarkup);
  bool ProcessInclude(std::string file, Include type);
  void ProcessDirectiveParsedLiteral();
  void ProcessDirectiveLiteralBlock();
  void ProcessDirectiveCodeBlock();
  void ProcessDirectiveReplace();
  void ProcessDirectiveTocTree();
  static void UnindentLines(std::vector<std::string>& lines);

  std::ostream& OS;
  std::string DocRoot;
  int IncludeDepth = 0;
  bool OutputLinePending = false;
  bool LastLineEndedInColonColon = false;
  Markup MarkupType = Markup::None;
  Directive DirectiveType = Directive::None;
  cmsys::RegularExpression CMakeDirective;
  cmsys::RegularExpression CMakeModuleDirective;
  cmsys::RegularExpression ParsedLiteralDirective;
  cmsys::RegularExpression CodeBlockDirective;
  cmsys::RegularExpression ReplaceDirective;
  cmsys::RegularExpression IncludeDirective;
  cmsys::RegularExpression TocTreeDirective;
  cmsys::RegularExpression ProductionListDirective;
  cmsys::RegularExpression NoteDirective;
  cmsys::RegularExpression VersionDirective;
  cmsys::RegularExpression ModuleRST;
  cmsys::RegularExpression CMakeRole;
  cmsys::RegularExpression InlineLink;
  cmsys::RegularExpression InlineLiteral;
  cmsys::RegularExpression Substitution;
  cmsys::RegularExpression TocTreeLink;

  std::vector<std::string> MarkupLines;
  std::string DocDir;
  std::map<std::string, std::string> Replace;
  std::set<std::string> Replaced;
  std::string ReplaceName;
};
