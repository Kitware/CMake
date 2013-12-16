/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmWIXPatchParser.h"

#include <CPack/cmCPackGenerator.h>

#include <cm_expat.h>

cmWIXPatchElement::~cmWIXPatchElement()
{
  for(child_list_t::iterator i = children.begin(); i != children.end(); ++i)
    {
    delete *i;
    }
}

cmWIXPatchParser::cmWIXPatchParser(
  fragment_map_t& fragments, cmCPackLog* logger):
    Logger(logger),
    state(BEGIN_DOCUMENT),
    valid(true),
    fragments(fragments)
{

}

void cmWIXPatchParser::StartElement(const char *name, const char **atts)
{
  std::string name_str = name;
  if(state == BEGIN_DOCUMENT)
    {
    if(name_str == "CPackWiXPatch")
      {
      state = BEGIN_FRAGMENTS;
      }
    else
      {
      ReportValidationError("Expected root element 'CPackWiXPatch'");
      }
    }
  else if(state == BEGIN_FRAGMENTS)
    {
      if(name_str == "CPackWiXFragment")
        {
        state = INSIDE_FRAGMENT;
        StartFragment(atts);
        }
      else
        {
        ReportValidationError("Expected 'CPackWixFragment' element");
        }
    }
  else if(state == INSIDE_FRAGMENT)
    {
      cmWIXPatchElement &parent = *elementStack.back();

      parent.children.resize(parent.children.size() + 1);
      cmWIXPatchElement*& currentElement = parent.children.back();
      currentElement = new cmWIXPatchElement;
      currentElement->name = name;

      for(size_t i = 0; atts[i]; i += 2)
        {
        std::string key = atts[i];
        std::string value = atts[i+1];

        currentElement->attributes[key] = value;
        }

      elementStack.push_back(currentElement);
    }
}

void cmWIXPatchParser::StartFragment(const char **attributes)
{
  for(size_t i = 0; attributes[i]; i += 2)
    {
    std::string key = attributes[i];
    std::string value = attributes[i+1];

    if(key == "Id")
      {
      if(fragments.find(value) != fragments.end())
        {
        std::stringstream tmp;
        tmp << "Invalid reuse of 'CPackWixFragment' 'Id': " << value;
        ReportValidationError(tmp.str());
        }

      elementStack.push_back(&fragments[value]);
      }
    else
      {
      ReportValidationError(
        "The only allowed 'CPackWixFragment' attribute is 'Id'");
      }
    }
}

void cmWIXPatchParser::EndElement(const char *name)
{
  std::string name_str = name;
  if(state == INSIDE_FRAGMENT)
    {
      if(name_str == "CPackWiXFragment")
        {
        state = BEGIN_FRAGMENTS;
        elementStack.clear();
        }
      else
        {
          elementStack.pop_back();
        }
    }
}

void cmWIXPatchParser::ReportError(int line, int column, const char* msg)
{
  cmCPackLogger(cmCPackLog::LOG_ERROR,
    "Error while processing XML patch file at " << line << ":" << column <<
      ":  "<< msg << std::endl);
  valid = false;
}

void cmWIXPatchParser::ReportValidationError(const std::string& message)
{
  ReportError(XML_GetCurrentLineNumber(Parser),
    XML_GetCurrentColumnNumber(Parser),
    message.c_str());
}

bool cmWIXPatchParser::IsValid() const
{
  return valid;
}
