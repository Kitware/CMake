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
#ifndef cmFunctionBlocker_h
#define cmFunctionBlocker_h

#include "cmStandardIncludes.h"
class cmMakefile;

/** \class cmFunctionBlocker
 * \brief A class that defines an interface for blocking cmake functions
 *
 * This is the superclass for any classes that need to block a cmake function
 */
class cmFunctionBlocker
{
public:
  /**
   * should a function be blocked
   */
  virtual bool IsFunctionBlocked(const char *name, const std::vector<std::string> &args, 
                                 const cmMakefile &mf) const = 0;

  /**
   * should this function blocker be removed, useful when one function adds a blocker
   * and another must remove it
   */
  virtual bool ShouldRemove(const char *name, const std::vector<std::string> &args, 
                            const cmMakefile &mf) const {return false;}
};

#endif
