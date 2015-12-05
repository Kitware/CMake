/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Matthias Maennich <matthias@maennich.net>
  Copyright 2010 Alexander Neundorf <neundorf@kde.org>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmParseArgumentsCommand.h"
#include "cmAlgorithms.h"

//----------------------------------------------------------------------------
bool cmParseArgumentsCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  // cmake_parse_arguments(prefix options single multi <ARGN>)
  //                         1       2      3      4
  if (args.size() < 4)
    {
    this->SetError("must be called with at least 4 arguments.");
    return false;
    }

  std::vector<std::string>::const_iterator argIter = args.begin(),
                                           argEnd  = args.end();
  // the first argument is the prefix
  const std::string prefix = (*argIter++) + "_";

  // define the result maps holding key/value pairs for
  // options, single values and multi values
  typedef std::map<std::string, bool> options_map;
  typedef std::map<std::string, std::string> single_map;
  typedef std::map<std::string, std::vector<std::string> > multi_map;
  options_map options;
  single_map single;
  multi_map multi;

  // anything else is put into a vector of unparsed strings
  std::vector<std::string> unparsed;

  // remember already defined keywords
  std::set<std::string> used_keywords;
  const std::string dup_warning = "keyword defined more than once: ";

  // the second argument is a (cmake) list of options without argument
  std::vector<std::string> list;
  cmSystemTools::ExpandListArgument(*argIter++, list);
  for (std::vector<std::string>::const_iterator iter  = list.begin(),
                                                end   = list.end();
                                                iter != end; ++iter)
    {
    if (!used_keywords.insert(*iter).second)
      {
      this->GetMakefile()->IssueMessage(cmake::WARNING, dup_warning + *iter);
      }
    options[*iter]; // default initialize
    }

  // the third argument is a (cmake) list of single argument options
  list.clear();
  cmSystemTools::ExpandListArgument(*argIter++, list);
  for (std::vector<std::string>::const_iterator iter  = list.begin(),
                                                end   = list.end();
                                                iter != end; ++iter)
    {
    if (!used_keywords.insert(*iter).second)
      {
      this->GetMakefile()->IssueMessage(cmake::WARNING, dup_warning + *iter);
      }
    single[*iter]; // default initialize
    }

  // the fourth argument is a (cmake) list of multi argument options
  list.clear();
  cmSystemTools::ExpandListArgument(*argIter++, list);
  for (std::vector<std::string>::const_iterator iter  = list.begin(),
                                                end   = list.end();
                                                iter != end; ++iter)
    {
    if (!used_keywords.insert(*iter).second)
      {
      this->GetMakefile()->IssueMessage(cmake::WARNING, dup_warning + *iter);
      }
    multi[*iter]; // default initialize
    }

  enum insideValues
  {
    NONE,
    SINGLE,
    MULTI
  } insideValues = NONE;
  std::string currentArgName;

  // now iterate over the remaining arguments
  // and fill in the values where applicable
  for(; argIter != argEnd; ++argIter)
    {
    const options_map::iterator optIter = options.find(*argIter);
    if (optIter != options.end())
      {
      insideValues = NONE;
      optIter->second = true;
      continue;
      }

    const single_map::iterator singleIter = single.find(*argIter);
    if (singleIter != single.end())
      {
      insideValues = SINGLE;
      currentArgName = *argIter;
      continue;
      }

    const multi_map::iterator multiIter = multi.find(*argIter);
    if (multiIter != multi.end())
      {
      insideValues = MULTI;
      currentArgName = *argIter;
      continue;
      }

    switch(insideValues)
      {
      case SINGLE:
        single[currentArgName] = *argIter;
        insideValues = NONE;
        break;
      case MULTI:
        multi[currentArgName].push_back(*argIter);
        break;
      default:
        unparsed.push_back(*argIter);
        break;
      }
    }

  // now iterate over the collected values and update their definition
  // within the current scope. undefine if necessary.

  for (options_map::const_iterator iter = options.begin(), end = options.end();
                                   iter != end; ++iter)
    {
    this->Makefile->AddDefinition(prefix + iter->first,
                                  iter->second? "TRUE": "FALSE");
    }
  for (single_map::const_iterator iter = single.begin(), end = single.end();
                                  iter != end; ++iter)
    {
    if (!iter->second.empty())
      {
      this->Makefile->AddDefinition(prefix + iter->first,
                                    iter->second.c_str());
      }
    else
      {
      this->Makefile->RemoveDefinition(prefix + iter->first);
      }
    }

  for (multi_map::const_iterator iter = multi.begin(), end = multi.end();
                                  iter != end; ++iter)
    {
    if (!iter->second.empty())
      {
      this->Makefile->AddDefinition(prefix + iter->first,
                                    cmJoin(cmMakeRange(iter->second), ";")
                                      .c_str());
      }
    else
      {
      this->Makefile->RemoveDefinition(prefix + iter->first);
      }
    }

  if (!unparsed.empty())
    {
    this->Makefile->AddDefinition(prefix + "UNPARSED_ARGUMENTS",
                                  cmJoin(cmMakeRange(unparsed), ";").c_str());
    }
  else
    {
    this->Makefile->RemoveDefinition(prefix + "UNPARSED_ARGUMENTS");
    }

  return true;
}
