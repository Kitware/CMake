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
    m_Elements.push_back(*arg);
    }
  
  return true;
}


/**
 * Write the CABIL configuration code to define this Set.
 */
void cmCabilDefineSetCommand::WriteConfiguration(std::ostream& os) const
{
  os << "  <Set name=\"" << m_SetName.c_str() << "\">" << std::endl;
  for(Elements::const_iterator e = m_Elements.begin();
      e != m_Elements.end(); ++e)
    {
    os << "    <Element>" << e->c_str() << "</Element>" << std::endl;
    }
  os << "  </Set>" << std::endl;
}
