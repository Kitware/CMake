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
#include "cmCableInstantiateCommand.h"
#include "cmCacheManager.h"

#include "cmRegularExpression.h"


/**
 * Write the CABLE configuration code to define this InstantiationSet.
 */
bool cmCableInstantiateCommand::WriteConfiguration()
{
  std::ostream& os = m_CableData->GetOutputStream();
  cmCableData::Indentation indent = m_CableData->GetIndentation();

  cmRegularExpression needCdataBlock("[&<>]");
  
  os << indent << "<InstantiationSet>" << std::endl;
  for(Entries::const_iterator e = m_Entries.begin();
      e != m_Entries.end(); ++e)
    {
    os << indent << "  <Element>";
    if(needCdataBlock.find(e->c_str()))
      {
      os << "<![CDATA[" << e->c_str() << "]]>";
      }
    else
      {
      os << e->c_str();
      }
    os << "</Element>" << std::endl;
    }
  os << indent << "</InstantiationSet>" << std::endl;
  
  return true;
}
