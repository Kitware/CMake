/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2013 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmRST.h"

#include "cmSystemTools.h"
#include "cmVersion.h"

#include <ctype.h>

//----------------------------------------------------------------------------
cmRST::cmRST(std::ostream& os, std::string const& docroot):
  OS(os),
  DocRoot(docroot),
  IncludeDepth(0),
  OutputLinePending(false),
  Markup(MarkupNone),
  Directive(DirectiveNone),
  CMakeDirective("^.. (cmake:)?("
                 "command|variable"
                 ")::[ \t]+([^ \t\n]+)$"),
  CMakeModuleDirective("^.. cmake-module::[ \t]+([^ \t\n]+)$"),
  ParsedLiteralDirective("^.. parsed-literal::[ \t]*(.*)$"),
  CodeBlockDirective("^.. code-block::[ \t]*(.*)$"),
  ReplaceDirective("^.. (\\|[^|]+\\|) replace::[ \t]*(.*)$"),
  IncludeDirective("^.. include::[ \t]+([^ \t\n]+)$"),
  TocTreeDirective("^.. toctree::[ \t]*(.*)$"),
  CMakeRole("(:cmake)?:("
            "command|generator|variable|module|policy|"
            "prop_cache|prop_dir|prop_gbl|prop_sf|prop_test|prop_tgt|"
            "manual"
            "):`(<*([^`<]|[^` \t]<)*)([ \t]+<[^`]*>)?`"),
  Substitution("(^|[^A-Za-z0-9_])"
               "((\\|[^| \t\r\n]([^|\r\n]*[^| \t\r\n])?\\|)(__|_|))"
               "([^A-Za-z0-9_]|$)")
{
  this->Replace["|release|"] = cmVersion::GetCMakeVersion();
}

//----------------------------------------------------------------------------
bool cmRST::ProcessFile(std::string const& fname, bool isModule)
{
  std::ifstream fin(fname.c_str());
  if(fin)
    {
    this->DocDir = cmSystemTools::GetFilenamePath(fname);
    if(isModule)
      {
      this->ProcessModule(fin);
      }
    else
      {
      this->ProcessRST(fin);
      }
    this->OutputLinePending = true;
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
void cmRST::ProcessRST(std::istream& is)
{
  std::string line;
  while(cmSystemTools::GetLineFromStream(is, line))
    {
    this->ProcessLine(line);
    }
  this->Reset();
}

//----------------------------------------------------------------------------
void cmRST::ProcessModule(std::istream& is)
{
  std::string line;
  std::string rst;
  while(cmSystemTools::GetLineFromStream(is, line))
    {
    if(rst == "#")
      {
      if(line == "#")
        {
        this->ProcessLine("");
        continue;
        }
      else if(line.substr(0, 2) == "# ")
        {
        this->ProcessLine(line.substr(2, line.npos));
        continue;
        }
      else
        {
        rst = "";
        this->Reset();
        this->OutputLinePending = true;
        }
      }
    if(line == "#.rst:")
      {
      rst = "#";
      }
    }
  if(rst == "#")
    {
    this->Reset();
    }
}

//----------------------------------------------------------------------------
void cmRST::Reset()
{
  if(!this->MarkupLines.empty())
    {
    this->UnindentLines(this->MarkupLines);
    }
  switch(this->Directive)
    {
    case DirectiveNone: break;
    case DirectiveParsedLiteral: this->ProcessDirectiveParsedLiteral(); break;
    case DirectiveCodeBlock: this->ProcessDirectiveCodeBlock(); break;
    case DirectiveReplace: this->ProcessDirectiveReplace(); break;
    case DirectiveTocTree: this->ProcessDirectiveTocTree(); break;
    }
  this->Markup = MarkupNone;
  this->Directive = DirectiveNone;
  this->MarkupLines.clear();
}

//----------------------------------------------------------------------------
void cmRST::ProcessLine(std::string const& line)
{
  // A line starting in .. is an explicit markup start.
  if(line == ".." || (line.size() >= 3 && line[0] == '.' &&
                      line[1] == '.' && isspace(line[2])))
    {
    this->Reset();
    this->Markup = (line.find_first_not_of(" \t", 2) == line.npos ?
                    MarkupEmpty : MarkupNormal);
    if(this->CMakeDirective.find(line))
      {
      // Output cmake domain directives and their content normally.
      this->NormalLine(line);
      }
    else if(this->CMakeModuleDirective.find(line))
      {
      // Process cmake-module directive: scan .cmake file comments.
      std::string file = this->CMakeModuleDirective.match(1);
      if(file.empty() || !this->ProcessInclude(file, IncludeModule))
        {
        this->NormalLine(line);
        }
      }
    else if(this->ParsedLiteralDirective.find(line))
      {
      // Record the literal lines to output after whole block.
      this->Directive = DirectiveParsedLiteral;
      this->MarkupLines.push_back(this->ParsedLiteralDirective.match(1));
      }
    else if(this->CodeBlockDirective.find(line))
      {
      // Record the literal lines to output after whole block.
      // Ignore the language spec and record the opening line as blank.
      this->Directive = DirectiveCodeBlock;
      this->MarkupLines.push_back("");
      }
    else if(this->ReplaceDirective.find(line))
      {
      // Record the replace directive content.
      this->Directive = DirectiveReplace;
      this->ReplaceName = this->ReplaceDirective.match(1);
      this->MarkupLines.push_back(this->ReplaceDirective.match(2));
      }
    else if(this->IncludeDirective.find(line))
      {
      // Process the include directive or output the directive and its
      // content normally if it fails.
      std::string file = this->IncludeDirective.match(1);
      if(file.empty() || !this->ProcessInclude(file, IncludeNormal))
        {
        this->NormalLine(line);
        }
      }
    else if(this->TocTreeDirective.find(line))
      {
      // Record the toctree entries to process after whole block.
      this->Directive = DirectiveTocTree;
      this->MarkupLines.push_back(this->TocTreeDirective.match(1));
      }
    }
  // An explicit markup start followed nothing but whitespace and a
  // blank line does not consume any indented text following.
  else if(this->Markup == MarkupEmpty && line.empty())
    {
    this->NormalLine(line);
    }
  // Indented lines following an explicit markup start are explicit markup.
  else if(this->Markup && (line.empty() || isspace(line[0])))
    {
    this->Markup = MarkupNormal;
    // Record markup lines if the start line was recorded.
    if(!this->MarkupLines.empty())
      {
      this->MarkupLines.push_back(line);
      }
    }
  // Print non-markup lines.
  else
    {
    this->NormalLine(line);
    }
}

//----------------------------------------------------------------------------
void cmRST::NormalLine(std::string const& line)
{
  this->Reset();
  this->OutputLine(line);
}

//----------------------------------------------------------------------------
void cmRST::OutputLine(std::string const& line_in)
{
  if(this->OutputLinePending)
    {
    this->OS << "\n";
    this->OutputLinePending = false;
    }
  std::string line = this->ReplaceSubstitutions(line_in);
  std::string::size_type pos = 0;
  while(this->CMakeRole.find(line.c_str()+pos))
    {
    this->OS << line.substr(pos, this->CMakeRole.start());
    std::string text = this->CMakeRole.match(3);
    // If a command reference has no explicit target and
    // no explicit "(...)" then add "()" to the text.
    if(this->CMakeRole.match(2) == "command" &&
       this->CMakeRole.match(5).empty() &&
       text.find_first_of("()") == text.npos)
      {
      text += "()";
      }
    this->OS << "``" << text << "``";
    pos += this->CMakeRole.end();
    }
  this->OS << line.substr(pos) << "\n";
}

//----------------------------------------------------------------------------
std::string cmRST::ReplaceSubstitutions(std::string const& line)
{
  std::string out;
  std::string::size_type pos = 0;
  while(this->Substitution.find(line.c_str()+pos))
    {
    std::string::size_type start = this->Substitution.start(2);
    std::string::size_type end = this->Substitution.end(2);
    std::string substitute = this->Substitution.match(3);
    std::map<cmStdString, cmStdString>::iterator
      replace = this->Replace.find(substitute);
    if(replace != this->Replace.end())
      {
      std::pair<std::set<cmStdString>::iterator, bool> replaced =
        this->Replaced.insert(substitute);
      if(replaced.second)
        {
        substitute = this->ReplaceSubstitutions(replace->second);
        this->Replaced.erase(replaced.first);
        }
      }
    out += line.substr(pos, start);
    out += substitute;
    pos += end;
    }
  out += line.substr(pos);
  return out;
}

//----------------------------------------------------------------------------
bool cmRST::ProcessInclude(std::string file, IncludeType type)
{
  bool found = false;
  if(this->IncludeDepth < 10)
    {
    cmRST r(this->OS, this->DocRoot);
    r.IncludeDepth = this->IncludeDepth + 1;
    r.OutputLinePending = this->OutputLinePending;
    if(type != IncludeTocTree)
      {
      r.Replace = this->Replace;
      }
    if(file[0] == '/')
      {
      file = this->DocRoot + file;
      }
    else
      {
      file = this->DocDir + "/" + file;
      }
    found = r.ProcessFile(file, type == IncludeModule);
    if(type != IncludeTocTree)
      {
      this->Replace = r.Replace;
      }
    this->OutputLinePending = r.OutputLinePending;
    }
  return found;
}

//----------------------------------------------------------------------------
void cmRST::ProcessDirectiveParsedLiteral()
{
  // Output markup lines as literal text.
  for(std::vector<std::string>::iterator i = this->MarkupLines.begin();
      i != this->MarkupLines.end(); ++i)
    {
    std::string line = *i;
    if(!line.empty())
      {
      line = " " + line;
      }
    this->OutputLine(line);
    }
  this->OutputLinePending = true;
}

//----------------------------------------------------------------------------
void cmRST::ProcessDirectiveCodeBlock()
{
  // Treat markup lines the same as a parsed literal.
  this->ProcessDirectiveParsedLiteral();
}

//----------------------------------------------------------------------------
void cmRST::ProcessDirectiveReplace()
{
  // Record markup lines as replacement text.
  std::string& replacement = this->Replace[this->ReplaceName];
  const char* sep = "";
  for(std::vector<std::string>::iterator i = this->MarkupLines.begin();
      i != this->MarkupLines.end(); ++i)
    {
    replacement += sep;
    replacement += *i;
    sep = " ";
    }
  this->ReplaceName = "";
}

//----------------------------------------------------------------------------
void cmRST::ProcessDirectiveTocTree()
{
  // Process documents referenced by toctree directive.
  for(std::vector<std::string>::iterator i = this->MarkupLines.begin();
      i != this->MarkupLines.end(); ++i)
    {
    if(!i->empty() && i->find_first_of(":") == i->npos)
      {
      this->ProcessInclude(*i + ".rst", IncludeTocTree);
      }
    }
}

//----------------------------------------------------------------------------
void cmRST::UnindentLines(std::vector<std::string>& lines)
{
  // Remove the common indentation from the second and later lines.
  std::string indentText;
  std::string::size_type indentEnd = 0;
  bool first = true;
  for(size_t i = 1; i < lines.size(); ++i)
    {
    std::string const& line = lines[i];

    // Do not consider empty lines.
    if(line.empty())
      {
      continue;
      }

    // Record indentation on first non-empty line.
    if(first)
      {
      first = false;
      indentEnd = line.find_first_not_of(" \t");
      indentText = line.substr(0, indentEnd);
      continue;
      }

    // Truncate indentation to match that on this line.
    if(line.size() < indentEnd)
      {
      indentEnd = line.size();
      }
    for(std::string::size_type j = 0; j != indentEnd; ++j)
      {
      if(line[j] != indentText[j])
        {
        indentEnd = j;
        break;
        }
      }
    }

  // Update second and later lines.
  for(size_t i = 1; i < lines.size(); ++i)
    {
    std::string& line = lines[i];
    if(!line.empty())
      {
      line = line.substr(indentEnd);
      }
    }

  // Drop leading blank lines.
  size_t leadingEmpty = 0;
  for(size_t i = 0; i < lines.size() && lines[i].empty(); ++i)
    {
    ++leadingEmpty;
    }
  lines.erase(lines.begin(), lines.begin()+leadingEmpty);

  // Drop trailing blank lines.
  size_t trailingEmpty = 0;
  for(size_t i = lines.size(); i > 0 && lines[i-1].empty(); --i)
    {
    ++trailingEmpty;
    }
  lines.erase(lines.end()-trailingEmpty, lines.end());
}
