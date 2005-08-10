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
    m_WhereSource = _("");
    m_WhereBuild = _("");
    m_AdvancedValues = false;
    m_GeneratorChoiceString.Empty();
    m_LastUnknownParameter = "";
    m_ValidArguments = "";
    m_ExitAfterLoad = false;
} 

///////////////////////////////////////////////////////////////
cmCommandLineInfo::~cmCommandLineInfo()
{
}

///////////////////////////////////////////////////////////////
bool cmCommandLineInfo::ParseCommandLine(int argc, char* argv[])
{
    bool result = true;
    wxString cachePath;
    
    // parse all commands
    int cc = 1;
    if(argc < cc)
        return true;    // no command line options

    while(cc < argc)
    {
        wxString arg = argv[cc];

        // if we have a switch
        if(arg.Len() > 1 && arg.GetChar(0) == '-')
        {
            int next_argc = ParseSwitch(argv, cc, argc);
            if(next_argc > 0)
                cc += next_argc;
            else
                return false; // sorry error while parsing
        }
        else
        {
            // gather all what is left
            for(int leftcc = cc; leftcc < argc; leftcc++)
            {
                if(cc != leftcc)
                    m_WhereBuild << _(" ");
                m_WhereBuild << argv[leftcc];
            }
            break;
        }
    }

    m_ExecutablePath = cmSystemTools::GetFilenamePath(argv[0]).c_str();

    return true;
}

///////////////////////////////////////////////////////////////
int cmCommandLineInfo::GetBoolValue(const wxString& v) {

    wxString value = v.Lower();
  
    if (!value.Cmp("1") || 
        !value.Cmp("on") || 
        !value.Cmp("true") || 
        !value.Cmp("yes"))
    {
        // true
        return 1;
    }
    else if (!value.Cmp("0") || 
             !value.Cmp("off") || 
             !value.Cmp("false") || 
             !value.Cmp("no"))
    {
        // false
        return -1;
    }

    // not recognised
    return 0;
}

///////////////////////////////////////////////////////////////
// Parse param

size_t cmCommandLineInfo::ParseSwitch(char **argv, int arg_index, int argc)
{        
    wxString param = argv[arg_index];
    
    // we need this for a switch, at least 2
    if(param.Len() > 1)
    {
        // determine switch type
        switch (param.GetChar(1))
        {
        case 'G':
            // when it's G<.....> we split else we take the 
            // other argc
            if(param.Len() > 2)
            {
                m_GeneratorChoiceString = GetStringParam(param.Mid(2));
                return 1;   // one arg is passed
            }
            else
            {
                if((arg_index+1) < argc)
                {
                    m_GeneratorChoiceString = GetStringParam(wxString(argv[arg_index+1]));
                    return 2;   // two args are passed
                }
            }
            // no luck
            return 0;
    
        case 'Q':
            m_ExitAfterLoad = true;
            return 1;

        // unknown param
        default:
            break;
        }
    }

    // error, unrecognised or too small arg
    return 0;
}

// When the string param given has string quotes around it
// we remove them and we pass back the string. If not, we 
// simply pass back the string as-is
wxString cmCommandLineInfo::GetStringParam(const wxString &str)
{
    wxString mystr = str.Strip(wxString::both);

    // if we have only one (or no chars return the current string)
    if(mystr.Len() < 2)
        return str;

    // if we have quotes
    if(mystr.GetChar(0) == '\"' && mystr.Last() == '\"')
    {
        // when we only have 2 in size, return empty string
        if(mystr.Len() == 2)
            return wxEmptyString;

        // now remove the outer and inner, and return
        return mystr.Mid(1, mystr.Len()-1);
    }

    return str;
}
