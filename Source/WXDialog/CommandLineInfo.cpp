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
    m_WhereSource = "";
    m_WhereBuild = "";
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
    
    for ( int cc = 1; cc < argc && result; cc ++ )
    {
        // skip (empty ???) params
        if ( strlen(argv[cc]) < 1 )
            continue;

        // judge argument and parse
        wxString argument(argv[cc]);
        if((argument.Len() > 1) && argument.GetChar(0) == '-')
            result = ParseArgument(argument.Mid(1));
        else
        {
            // ok this is the last of the arguments, the rest of the string(s)
            // we concatenate to the cache path or something else
            if(cc > 1)
                cachePath << " ";
            cachePath << argument;
        }
    }
  
    m_ExecutablePath = cmSystemTools::GetFilenamePath(argv[0]).c_str();

    return result;
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

bool cmCommandLineInfo::ParseArgument(const wxString& sParam)
{    
    bool result = false;
    
    if(sParam.Len() > 1)
    {
        wxString value = sParam.Mid(1);
        switch (sParam[0])
        {
            case 'G':
                m_GeneratorChoiceString = GetStringParam(value);
                result = true;
                break;
        
            case 'Q':
                m_ExitAfterLoad = true;
                result = true;
                break;

            // unknown param
            default:
                break;
        }
    }

    return result;
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
