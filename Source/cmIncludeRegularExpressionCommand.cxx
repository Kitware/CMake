/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmIncludeRegularExpressionCommand.h"

// cmIncludeRegularExpressionCommand
bool cmIncludeRegularExpressionCommand::InitialPass(std::vector<std::string> const& args)
{
  if((args.size() < 1) || (args.size() > 2))
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  m_Makefile->SetIncludeRegularExpression(args[0].c_str());
  
  if(args.size() > 1)
    {
    m_Makefile->SetComplainRegularExpression(args[1].c_str());
    }
  
  return true;
}

