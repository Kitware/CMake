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
#include "cmCableSourceFilesCommand.h"
#include "cmCacheManager.h"

/**
 * Write the CABLE configuration code to indicate header dependencies for
 * a package.
 */
void cmCableSourceFilesCommand::WriteConfiguration() const
{
  std::ostream& os = m_CableData->GetOutputStream();
  cmCableData::Indentation indent = m_CableData->GetIndentation();
  
  cmRegularExpression needCdataBlock("[&<>]");
  
  os << indent << "<Headers>" << std::endl;
  for(Entries::const_iterator f = m_Entries.begin();
      f != m_Entries.end(); ++f)
    {
    os << indent << "  <File name=\"" << f->c_str() << ".h\"/>" << std::endl;
    }
  os << indent << "</Headers>" << std::endl;
}
