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
#include "cmIfCommand.h"
#include "cmCacheManager.h"

bool cmIfFunctionBlocker::
IsFunctionBlocked(const char *name, const std::vector<std::string> &args, 
                  cmMakefile &)
{
  if (!strcmp(name,"ELSE") || !strcmp(name,"ENDIF"))
    {
    if (args == m_Args)
      {
      return false;
      }
    else if(args.empty())
      {
      std::string err = "Empty arguments for ";
      err += name;
      err += ".  Did you mean ";
      err += name;
      err += "( ";
      for(std::vector<std::string>::const_iterator a = m_Args.begin();
          a != m_Args.end();++a)
        {
        err += *a;
        err += " ";
        }
      err += ")?";
      cmSystemTools::Error(err.c_str());
      }
    }
  return true;
}

bool cmIfFunctionBlocker::
ShouldRemove(const char *name, const std::vector<std::string> &args, 
             cmMakefile &mf) 
{
  return !this->IsFunctionBlocked(name,args,mf);
}

void cmIfFunctionBlocker::
ScopeEnded(cmMakefile &mf)
{
  std::string errmsg = "The end of a CMakeLists file was reached with an IF statement that was not closed properly.\nWithin the directory: ";
  errmsg += mf.GetCurrentDirectory();
  errmsg += "\nThe arguments are: ";
  for(std::vector<std::string>::const_iterator j = m_Args.begin();
      j != m_Args.end(); ++j)
    {   
    errmsg += *j;
    errmsg += " ";
    }
  cmSystemTools::Error(errmsg.c_str());
}

bool cmIfCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  // create a function blocker
  cmIfFunctionBlocker *f = NULL;

  // check for the different signatures
  const char *def;
  const char *def2;

  if (args.size() == 1)
    {
    def = m_Makefile->GetDefinition(args[0].c_str());
    if(cmSystemTools::IsOff(def))
      {
      f = new cmIfFunctionBlocker();
      }
    }

  if (args.size() == 2 && (args[0] == "NOT"))
    {
    def = m_Makefile->GetDefinition(args[1].c_str());
    if(!cmSystemTools::IsOff(def))
      {
      f = new cmIfFunctionBlocker();
      }
    }

  if (args.size() == 2 && (args[0] == "COMMAND"))
    {
    if(!m_Makefile->CommandExists(args[1].c_str()))
      {
      f = new cmIfFunctionBlocker();
      }
    }

  if (args.size() == 2 && (args[0] == "EXISTS"))
    {
    if(!cmSystemTools::FileExists(args[1].c_str()))
      {
      f = new cmIfFunctionBlocker();
      }
    }

  if (args.size() == 3 && (args[1] == "AND"))
    {
    def = m_Makefile->GetDefinition(args[0].c_str());
    def2 = m_Makefile->GetDefinition(args[2].c_str());
    if(cmSystemTools::IsOff(def) || cmSystemTools::IsOff(def2))
      {
      f = new cmIfFunctionBlocker();
      }
    }
  
  if (args.size() == 3 && (args[1] == "OR"))
    {
    def = m_Makefile->GetDefinition(args[0].c_str());
    def2 = m_Makefile->GetDefinition(args[2].c_str());
    if(cmSystemTools::IsOff(def) && cmSystemTools::IsOff(def2))
      {
      f = new cmIfFunctionBlocker();
      }
    }

  if (args.size() == 3 && (args[1] == "MATCHES"))
    {
    def = m_Makefile->GetDefinition(args[0].c_str());
    cmRegularExpression regEntry(args[2].c_str());
    
    // check for black line or comment
    if (!def || !regEntry.find(def))
      {
      f = new cmIfFunctionBlocker();
      }
    }
  
  // if we created a function blocker then set its args
  if (f)
    {
    for(std::vector<std::string>::const_iterator j = args.begin();
        j != args.end(); ++j)
      {   
      f->m_Args.push_back(*j);
      }
    m_Makefile->AddFunctionBlocker(f);
    }
  
  return true;
}

