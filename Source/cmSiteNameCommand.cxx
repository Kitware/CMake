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
#include "cmSiteNameCommand.h"

// cmSiteNameCommand
bool cmSiteNameCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  const char* cacheValue
    = m_Makefile->GetDefinition(args[0].c_str());
  if(cacheValue)
    {
    return true;
    }
  
  const char *temp = m_Makefile->GetDefinition("HOSTNAME");
  std::string hostname_cmd;
  if(temp)
    {
    hostname_cmd = temp;
    }
  else 
    {
    hostname_cmd = cmSystemTools::FindProgram("hostname");
    }
  
  std::string siteName = "unknown";
  
  // try to find the hostname for this computer
  if (hostname_cmd.length()) 
    {
    std::string host;
    cmSystemTools::RunCommand(hostname_cmd.c_str(),
                              host);
    
    // got the hostname
    if (host.length())
      {
      // remove any white space from the host name
      std::string hostRegExp = "[ \t\n\r]*([^\t\n\r ]*)[ \t\n\r]*";
      cmRegularExpression hostReg (hostRegExp.c_str());
      if (hostReg.find(host.c_str()))
        {
        // strip whitespace
        host = hostReg.match(1);
        }

      if(host.length())
        {
        siteName = host;

        temp = m_Makefile->GetDefinition("NSLOOKUP");
        std::string nslookup_cmd;
        if(temp)
          {
          nslookup_cmd = temp;
          }
        else
          {
          nslookup_cmd = cmSystemTools::FindProgram("nslookup");
          }

        // try to find the domain name for this computer
        if (nslookup_cmd.length())
          {
          nslookup_cmd += " ";
          nslookup_cmd += host;
          std::string nsOutput;
          cmSystemTools::RunCommand(nslookup_cmd.c_str(),
                                    nsOutput);

          // got the domain name
          if (nsOutput.length())
            {
            std::string RegExp = ".*Name:[ \t\n]*";
            RegExp += host;
            RegExp += "\\.([^ \t\n\r]*)[ \t\n\r]*Address:";
            cmRegularExpression reg( RegExp.c_str() );
            if(reg.find(nsOutput.c_str()))
              {
              siteName += '.' + cmSystemTools::LowerCase(reg.match(1));
              }
            }
          }
        }
      }
    }
  
  m_Makefile->
    AddCacheDefinition(args[0].c_str(),
                       siteName.c_str(),
                       "Name of the computer/site where compile is being run",
                       cmCacheManager::STRING);

  return true;
}

