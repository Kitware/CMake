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
#include "cmCabilDefineSetCommand.h"
#include "cmCacheManager.h"

#include "cmRegularExpression.h"


// cmCabilDefineSetCommand
bool cmCabilDefineSetCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  std::vector<std::string>::const_iterator arg = args.begin();
  
  // The first argument is the name of the set.
  m_SetName = *arg++;
  
  // The rest of the arguments are the elements to be placed in the set.
  for(; arg != args.end(); ++arg)
    {
    m_Elements.push_back(Element(this->GenerateTag(*arg), *arg));
    }
  
  return true;
}


/**
 * Write the CABIL configuration code to define this Set.
 */
void cmCabilDefineSetCommand::WriteConfiguration(std::ostream& os) const
{
  cmRegularExpression needCdataBlock("[&<>]");
  
  os << "  <Set name=\"" << m_SetName.c_str() << "\">" << std::endl;
  for(Elements::const_iterator e = m_Elements.begin();
      e != m_Elements.end(); ++e)
    {
    os << "    <Element";
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
  os << "  </Set>" << std::endl;
}


/**
 * Given the string representing a set element, automatically generate
 * the CABIL element tag for it.
 *
 * **This function determines how the output language of all
 * CABIL-generated wrappers will look!**
 */
std::string
cmCabilDefineSetCommand::GenerateTag(const std::string& element) const
{
  // Hold the regular expressions for matching against the element.
  cmRegularExpression regex;
  
  // If the element's code begins in a $, it is referring to a set name.
  // The set's elements have their own tags, so we don't need one.
  regex.compile("^[ \t]*\\$");
  if(regex.find(element))
    { return ""; }
  
  // Test for simple integer
  regex.compile("^[ \t]*([0-9]*)[ \t]*$");
  if(regex.find(element))
    {
    std::string tag = "_";
    tag.append(regex.match(1));
    return tag;
    }

  // Test for basic integer type
  regex.compile("^[ \t]*(unsigned[ ]|signed[ ])?[ \t]*(char|short|int|long|long[ ]long)[ \t]*$");
  if(regex.find(element))
    {
    std::string tag = "_";
    if(regex.match(1) == "unsigned ")
      { tag.append("u"); }
    if(regex.match(2) == "long long")
      { tag.append("llong"); }
    else
      { tag.append(regex.match(2)); }
    return tag;
    }

  // Test for basic floating-point type
  regex.compile("^[ \t]*(long[ ])?[ \t]*(float|double)[ \t]*$");
  if(regex.find(element))
    {
    std::string tag = "_";
    if(regex.match(1) == "long ")
      tag.append("l");
    tag.append(regex.match(2));
    return tag;
    }

  // Test for basic wide-character type
  regex.compile("^[ \t]*(wchar_t)[ \t]*$");
  if(regex.find(element))
    {
    return "_wchar";
    }

  // Test for plain type name (without template arguments).
  regex.compile("^[ \t]*([A-Za-z_][A-Za-z0-9_]*)[ \t]*$");
  if(regex.find(element))
    {
    // The tag is the same as the type.
    return regex.match(1);
    }
  
  // Test for template class instance.
  regex.compile("^[ \t]*([A-Za-z_][A-Za-z0-9_]*)<.*[ \t]*$");
  if(regex.find(element))
    {
    // The tag is the type without arguments (the arguments may have
    // their own tags).
    return regex.match(1);
    }
  
  return "NO_AUTO_TAG";
}
