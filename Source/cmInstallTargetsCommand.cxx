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
#include "cmInstallTargetsCommand.h"
#include "cmCacheManager.h"

// cmExecutableCommand
bool cmInstallTargetsCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  cmTargets &tgts = m_Makefile->GetTargets();
  std::vector<std::string>::const_iterator s = args.begin();
  ++s;
  for (;s != args.end(); ++s)
    {
    if (tgts.find(*s) != tgts.end())
      {
      tgts[*s].SetInstallPath(args[0].c_str());
      }
    }
  
  return true;
}

