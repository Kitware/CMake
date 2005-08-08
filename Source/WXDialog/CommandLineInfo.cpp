/*=========================================================================

  Program:   WXDialog - wxWidgets X-platform GUI Front-End for CMake
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Author:    Jorgen Bodde

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "CommandLineInfo.h" 

#include "cmSystemTools.h"

///////////////////////////////////////////////////////////////
// cmCommandLineInfo 

cmCommandLineInfo::cmCommandLineInfo()
{
  this->m_WhereSource = "";
  this->m_WhereBuild = "";
  this->m_AdvancedValues = false;
  m_GeneratorChoiceString.Empty();
  this->m_LastUnknownParameter = "";
  this->m_ValidArguments = "";
  this->m_ExitAfterLoad = false;
} 

///////////////////////////////////////////////////////////////
cmCommandLineInfo::~cmCommandLineInfo()
{
}

///////////////////////////////////////////////////////////////
void cmCommandLineInfo::ParseCommandLine(int argc, char* argv[])
{
    for ( int cc = 1; cc < argc; cc ++ )
    {
        if ( strlen(argv[cc]) < 1 )
            continue;

        std::string argument = argv[cc];
        bool valid = ((argument.size() > 1) && (m_ValidArguments.find(argument[1]) == std::string::npos));
        
        ParseParam(argument, valid, (cc + 1 == argc));
    }
  
    m_ExecutablePath = cmSystemTools::GetFilenamePath(argv[0]);
}

///////////////////////////////////////////////////////////////
int cmCommandLineInfo::GetBoolValue(const std::string& v) {
  std::string value = cmSystemTools::LowerCase(v);
  if (value == "1" || 
      value == "on" || 
      value == "true" || 
      value == "yes")
    {
    return 1;
    }
  else if (value == "0" || 
           value == "off" || 
           value == "false" || 
           value == "no")
    {
    return -1;
    }
  return 0;
}

///////////////////////////////////////////////////////////////
// Parse param

void cmCommandLineInfo::ParseParam(const std::string& parameter, 
                                   bool know_about, bool /*last*/)
{
    // this is the last parameter we know, so we assign this to be
    // path to source or path to existing build
    if(!know_about)
        m_LastUnknownParameter = parameter;
    else
    {
        std::string sParam(parameter.c_str(), 1, parameter.npos);
        
        // Single letter valued flag like /B=value or /B:value
        std::string value;
        if (sParam[1] == '=' || sParam[1] == ':')
        {
            value = std::string(parameter.c_str()+3);
        }
        else
        {
            value = std::string(parameter.c_str()+2);
        }
        int res;
    
        switch (sParam[0])
        {
            case 'A':
                res = cmCommandLineInfo::GetBoolValue(value);
                if (res == 1)
                {
                    m_AdvancedValues = true;
                }
                else if (res == -1)
                {
                    m_AdvancedValues = false;
                }
                break;
      
            case 'B':
                m_WhereBuild = value;
                break;
      
            case 'G':
                m_GeneratorChoiceString = GetStringParam(value.c_str());
                break;
      
            case 'Q':
                m_ExitAfterLoad = true;
                break;
            
            case 'H':
                m_WhereSource = value;
                break;
        }
    }
}

// When the string param given has string quotes around it
// we remove them and we pass back the string. If not, we 
// simply pass back the string as-is
wxString cmCommandLineInfo::GetStringParam(const char *pString)
{
    wxCHECK(pString, wxEmptyString);

    wxString str(pString);
    str = str.Strip(wxString::both);

    // if we have only one (or no chars return the current string)
    if(str.Len() < 2)
        return str;

    // if we have quotes
    if(str.GetChar(0) == '\"' && str.Last() == '\"')
    {
        // when we only have 2 in size, return empty string
        if(str.Len() == 2)
            return wxEmptyString;

        // now remove the outer and inner, and return
        return str.Mid(1, str.Len()-1);
    }

    return str;
}
