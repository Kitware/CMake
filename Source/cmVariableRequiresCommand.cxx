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
#include "cmVariableRequiresCommand.h"
#include "cmCacheManager.h"

// cmLibraryCommand
bool cmVariableRequiresCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  cmSystemTools::ExpandListArguments(args, m_Arguments);
  return true;
}

void cmVariableRequiresCommand::FinalPass()
{
  std::string testVarible = m_Arguments[0]; 
  if(!m_Makefile->IsOn(testVarible.c_str()))
    {
    return;
    }
  std::string resultVarible = m_Arguments[1];
  bool requirementsMet = true;
  std::string notSet;
  bool hasAdvanced = false;
  for(unsigned int i = 2; i < m_Arguments.size(); ++i)
    {
    if(!m_Makefile->IsOn(m_Arguments[i].c_str()))
      {
      requirementsMet = false;
      notSet += m_Arguments[i];
      notSet += "\n";
      if(m_Makefile->GetCacheManager()->IsAdvanced(m_Arguments[i].c_str()))
        {
        hasAdvanced = true;
        }
      }
    }
  const char* reqVar = m_Makefile->GetDefinition(resultVarible.c_str());
  // if reqVar is unset, then set it to requirementsMet 
  // if reqVar is set to true, but requirementsMet is false , then
  // set reqVar to false.
  if(!reqVar || (!requirementsMet && m_Makefile->IsOn(reqVar)))
    {
    m_Makefile->AddDefinition(resultVarible.c_str(), requirementsMet);
    }

  if(!requirementsMet)
    {
    std::string message = "Variable assertion failed:\n";
    message += testVarible + " Requires that the following unset varibles are set:\n";
    message += notSet;
    message += "\nPlease set them, or set ";
    message += testVarible + " to false, and re-configure.\n";
    if(hasAdvanced)
      {
      message += "One or more of the required variables is advanced.  To set the variable, you must turn on advanced mode in cmake.";
      }
    cmSystemTools::Error(message.c_str());
    }
}
