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
#include "cmElseCommand.h"
#include "cmCacheManager.h"

bool cmElseCommand::InitialPass(std::vector<std::string> const& args)
{
  bool isValid;
  bool isTrue = cmIfCommand::IsTrue(args,isValid,m_Makefile);
  
  if (!isValid)
    {
    this->SetError("An ELSE command had incorrect arguments");
    return false;
    }
  
  // first remove any function blockers for the IF
  m_Makefile->RemoveFunctionBlocker("ELSE",args);

  // if is true create a blocker for the else
  cmIfFunctionBlocker *f = new cmIfFunctionBlocker();
  f->m_IsBlocking = isTrue;
  for(std::vector<std::string>::const_iterator j = args.begin();
      j != args.end(); ++j)
    {   
    f->m_Args.push_back(*j);
    }
  m_Makefile->AddFunctionBlocker(f);
  
  return true;
}

