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
#ifndef cmCommands_h
#define cmCommands_h
#include "cmStandardIncludes.h"

class cmCommand;
/**
 * Global function to return all compiled in commands.
 * To add a new command edit cmCommands.cxx and add your command.
 * It is up to the caller to delete the commands created by this
 * call.
 */
void GetPredefinedCommands(std::list<cmCommand*>& commands);


#endif
