/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "cmCableInstantiateClassCommand.h"
#include "cmCacheManager.h"

#include "cmRegularExpression.h"


/**
 * Write the CABLE configuration code to define this InstantiationSet.
 * This includes the "class" keyword to do class template instantiations.
 */
void cmCableInstantiateClassCommand::WriteConfiguration() const
{
  std::ostream& os = m_CableData->GetOutputStream();
  cmCableData::Indentation indent = m_CableData->GetIndentation();

  cmRegularExpression needCdataBlock("[&<>]");
  
  os << indent << "<InstantiationSet>" << std::endl;
  for(Entries::const_iterator e = m_Entries.begin();
      e != m_Entries.end(); ++e)
    {
    os << indent << "  <Element>class ";
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
}
