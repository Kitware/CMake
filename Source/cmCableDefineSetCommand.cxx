/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#include "cmCableDefineSetCommand.h"
#include "cmCacheManager.h"

#include "cmRegularExpression.h"


// cmCableDefineSetCommand
bool cmCableDefineSetCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // This command needs access to the Cable data.
  this->SetupCableData();
  
  std::vector<std::string>::const_iterator arg = args.begin();
  
  // The first argument is the name of the set.
  m_SetName = *arg++;
  
  // All arguments until a "SOURCE_FILES" are the elements to be placed in
  // the set.
  for(; (arg != args.end()) && (*arg != "SOURCE_FILES"); ++arg)
    {
    // If the element cannot be added, return an error.
    // This can occur when a tag is not specified and can't be generated.
    if(!this->AddElement(*arg))
      { return false; }
    }

  // If we are not at the end, the "SOURCE_FILES" keyword has been
  // encountered.
  if(arg != args.end())
    {
    // The rest of the arguments are source files to be included in
    // any package which references the set.
    for(++arg; arg != args.end(); ++arg)
      {
      if(!this->AddSourceFile(*arg))
        { return false; }
      }
    }
  
  // Write this command's configuration output.
  this->WriteConfiguration();
  
  return true;
}


/**
 * Write the CABLE configuration code to define this Set.
 */
void cmCableDefineSetCommand::WriteConfiguration() const
{
  cmRegularExpression needCdataBlock("[&<>]");
  
  // Get the ouptut information from the cmCableData.
  std::ostream& os = m_CableData->GetOutputStream();
  cmCableData::Indentation indent = m_CableData->GetIndentation();
  
  // Output the code.
  os << indent << "<Set name=\"" << m_SetName.c_str() << "\">" << std::endl;
  for(std::vector<std::string>::const_iterator e = m_SourceHeaders.begin();
      e != m_SourceHeaders.end(); ++e)
    {
    os << indent << "  <File name=\"" << e->c_str() << "\"/>" << std::endl;
    }
  for(std::vector<std::string>::const_iterator e = m_InstantiationSources.begin();
      e != m_InstantiationSources.end(); ++e)
    {
    os << indent << "  <File name=\"" << e->c_str()
       << "\" purpose=\"instantiate\"/>" << std::endl;
    }
  for(Elements::const_iterator e = m_Elements.begin();
      e != m_Elements.end(); ++e)
    {
    os << indent << "  <Element";
    // Only output the tag if it is not the empty string.
    if(e->first.length() > 0)
      {
      os << " tag=\"" << e->first.c_str() << "\"";
      }
    os << ">";
    if(needCdataBlock.find(e->second.c_str()))
      {
      os << "<![CDATA[" << e->second.c_str() << "]]>";
      }
    else
      {
      os << e->second.c_str();
      }
    os << "</Element>" << std::endl;
    }
  os << indent << "</Set>" << std::endl;
}


/**
 * Add an element to the set.  The given string is the argument to the
 * command describing the element.  There are two formats allowed:
 *  "code" = The code describing the element to CABLE is simply given.
 *           The GenerateTag() method will guess at a good tag for the
 *           code.
 *  "tag:code" = The left side of a single colon is text describing the tag.
 *               GenerateTag() will not be called.
 */
bool cmCableDefineSetCommand::AddElement(const std::string& arg)
{
  // A regular expression to match the tagged element specification.
  cmRegularExpression tagGiven("^([A-Za-z_0-9]*)[ \t]*:[ \t]*([^:].*|::.*)$");
  
  std::string tag;
  std::string code;

  if(tagGiven.find(arg.c_str()))
    {
    // A tag was given.  Use it.
    tag = tagGiven.match(1);
    code = tagGiven.match(2);
    }
  else
    {
    // No tag was given.  Try to generate one.
    if(!this->GenerateTag(arg, tag))
      { return false; }
    code = arg;
    }
  
  // Add an element with the given tag and code.
  m_Elements.push_back(Element(tag, code));
  return true;
}


/**
 * Given the string representing a set element, automatically generate
 * the CABLE element tag for it.
 *
 * **This function determines how the output language of all
 * CABLE-generated wrappers will look!**
 */
bool
cmCableDefineSetCommand::GenerateTag(const std::string& element,
                                     std::string& tag)
{
  // Hold the regular expressions for matching against the element.
  cmRegularExpression regex;
  
  // If the element's code begins in a $, it is referring to a set name.
  // The set's elements have their own tags, so we don't need one.
  regex.compile("^[ \t]*\\$");
  if(regex.find(element))
    { tag = ""; return true; }
  
  // Test for simple integer
  regex.compile("^[ \t]*([0-9]*)[ \t]*$");
  if(regex.find(element))
    {
    tag = "_";
    tag.append(regex.match(1));
    return true;
    }

  // Test for basic integer type
  regex.compile("^[ \t]*(unsigned[ ]|signed[ ])?[ \t]*(char|short|int|long|long[ ]long)[ \t]*$");
  if(regex.find(element))
    {
    tag = "_";
    if(regex.match(1) == "unsigned ")
      { tag.append("u"); }
    if(regex.match(2) == "long long")
      { tag.append("llong"); }
    else
      { tag.append(regex.match(2)); }
    return true;
    }

  // Test for basic floating-point type
  regex.compile("^[ \t]*(long[ ])?[ \t]*(float|double)[ \t]*$");
  if(regex.find(element))
    {
    tag = "_";
    if(regex.match(1) == "long ")
      tag.append("l");
    tag.append(regex.match(2));
    return true;
    }

  // Test for basic wide-character type
  regex.compile("^[ \t]*(wchar_t)[ \t]*$");
  if(regex.find(element))
    {
    tag = "_wchar";
    return true;
    }

  // Test for plain type name (without template arguments).
  regex.compile("^[ \t]*([A-Za-z_][A-Za-z0-9_]*)[ \t]*$");
  if(regex.find(element))
    {
    // The tag is the same as the type.
    tag = regex.match(1);
    return true;
    }
  
  // Test for template class instance.
  regex.compile("^[ \t]*([A-Za-z_][A-Za-z0-9_]*)<.*[ \t]*$");
  if(regex.find(element))
    {
    // The tag is the type without arguments (the arguments may have
    // their own tags).
    tag = regex.match(1);
    return true;
    }
  
  // We can't generate a tag.
  std::string err =
    ("doesn't know how to generate tag for element \""+element+"\" in set \""
     +m_SetName+"\"\nPlease specify one with the \"tag:element\" syntax.");
  this->SetError(err.c_str());

  tag = "";
  
  return false;
}


/**
 * Add a source file associated with this set.  Any package referencing
 * this set will automatically include this source file.
 */
bool cmCableDefineSetCommand::AddSourceFile(const std::string& file)
{
  // We must locate the file in the include path so that we can detect
  // its extension, and whether there is more than one to find.
  std::string header = file+".h";
  std::string txx = file+".txx";
  m_Makefile->ExpandVariablesInString(header);
  m_Makefile->ExpandVariablesInString(txx);
      
  // See if the file just exists here.  The compiler's search path will
  // locate it.
  if(cmSystemTools::FileExists(header.c_str()))
    {
    m_SourceHeaders.push_back(header);
    // See if there is a matching .txx as well.
    if(cmSystemTools::FileExists(txx.c_str()))
      {
      m_InstantiationSources.push_back(txx);
      }
    return true;
    }
  
  // We must look for the file in the include search path.
  const std::vector<std::string>& includeDirectories =
    m_Makefile->GetIncludeDirectories();
  
  for(std::vector<std::string>::const_iterator dir = includeDirectories.begin();
      dir != includeDirectories.end(); ++dir)
    {
    std::string path = *dir + "/";
    m_Makefile->ExpandVariablesInString(path);
    if(cmSystemTools::FileExists((path+header).c_str()))
      {
      m_SourceHeaders.push_back(header);
      // See if there is a matching .txx as well.
      if(cmSystemTools::FileExists((path+txx).c_str()))
        {
        m_InstantiationSources.push_back(txx);
        }
      return true;
      }
    }

  // We couldn't locate the source file.  Report the error.
  std::string err = "couldn't find source file " + header;
  this->SetError(err.c_str());
  return false;
}
