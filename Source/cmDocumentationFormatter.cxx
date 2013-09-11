/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmDocumentationFormatter.h"

cmDocumentationFormatter::cmDocumentationFormatter()
{
}

cmDocumentationFormatter::~cmDocumentationFormatter()
{
}

void cmDocumentationFormatter::PrintFormatted(std::ostream& os,
                                              const char* text)
{
  if(!text)
    {
    return;
    }
  const char* ptr = text;
  while(*ptr)
    {
    // Any ptrs starting in a space are treated as preformatted text.
    std::string preformatted;
    while(*ptr == ' ')
      {
      for(char ch = *ptr; ch && ch != '\n'; ++ptr, ch = *ptr)
        {
        preformatted.append(1, ch);
        }
      if(*ptr)
        {
        ++ptr;
        preformatted.append(1, '\n');
        }
      }
    if(preformatted.length())
      {
      this->PrintPreformatted(os, preformatted.c_str());
      }

    // Other ptrs are treated as paragraphs.
    std::string paragraph;
    for(char ch = *ptr; ch && ch != '\n'; ++ptr, ch = *ptr)
      {
      paragraph.append(1, ch);
      }
    if(*ptr)
      {
      ++ptr;
      paragraph.append(1, '\n');
      }
    if(paragraph.length())
      {
      this->PrintParagraph(os, paragraph.c_str());
      }
    }
}

//----------------------------------------------------------------------------
std::string
cmDocumentationFormatter::ComputeSectionLinkPrefix(std::string const& name)
{
  // Map from section name to a prefix for links pointing within the
  // section.  For example, the commands section should have HTML
  // links to each command with names like #command:ADD_EXECUTABLE.
  if(name.find("Command") != name.npos)
    {
    return "command";
    }
  else if(name.find("Propert") != name.npos)
    {
    if(name.find("Global") != name.npos)
      {
      return "prop_global";
      }
    else if(name.find("Direct") != name.npos)
      {
      return "prop_dir";
      }
    else if(name.find("Target") != name.npos)
      {
      return "prop_tgt";
      }
    else if(name.find("Test") != name.npos)
      {
      return "prop_test";
      }
    else if(name.find("Source") != name.npos)
      {
      return "prop_sf";
      }
    return "property";
    }
  else if(name.find("Variable") != name.npos)
    {
    return "variable";
    }
  else if(name.find("Polic") != name.npos)
    {
    return "policy";
    }
  else if(name.find("Module") != name.npos)
    {
    return "module";
    }
  else if(name.find("Name") != name.npos ||
          name.find("Introduction") != name.npos)
    {
    return "name";
    }
  else if(name.find("Usage") != name.npos)
    {
    return "usage";
    }
  else if(name.find("Description") != name.npos)
    {
    return "desc";
    }
  else if(name.find("Generators") != name.npos)
    {
    return "gen";
    }
  else if(name.find("Options") != name.npos)
    {
    return "opt";
    }
  else if(name.find("Copyright") != name.npos)
    {
    return "copy";
    }
  else if(name.find("See Also") != name.npos)
    {
    return "see";
    }
  else if(name.find("SingleItem") != name.npos)
    {
    return "single_item";
    }
  else if(name.find("Concepts") != name.npos)
    {
    return "concept";
    }
  else
    {
    std::cerr
      << "WARNING: ComputeSectionLinkPrefix failed for \"" << name << "\""
      << std::endl;
    return "other";
    }
}
