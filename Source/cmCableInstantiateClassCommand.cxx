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
#include "cmCableInstantiateClassCommand.h"
#include "cmCacheManager.h"

#include "cmRegularExpression.h"


/**
 * Write the CABLE configuration code to define this InstantiationSet.
 * This includes the "class" keyword to do class template instantiations.
 */
void cmCableInstantiateClassCommand::WriteConfiguration(std::ostream& os) const
{
  cmRegularExpression needCdataBlock("[&<>]");
  
  os << std::endl
     << "  <InstantiationSet>" << std::endl;
  for(Elements::const_iterator e = m_Elements.begin();
      e != m_Elements.end(); ++e)
    {
    os << "    <Element>class ";
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
  os << "  </InstantiationSet>" << std::endl;
}
