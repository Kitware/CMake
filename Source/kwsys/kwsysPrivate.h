/*=========================================================================

  Program:   KWSys - Kitware System Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef KWSYS_NAMESPACE
# error "Do not include kwsysPrivate.h outside of kwsys c and cxx files."
#endif

#ifndef _kwsysPrivate_h
#define _kwsysPrivate_h

/*
  Define KWSYS_HEADER macro to help the c and cxx files include kwsys
  headers from the configured namespace directory.  The macro can be
  used like this:
  
  #include KWSYS_HEADER(Directory.hxx)
  #include KWSYS_HEADER(std/vector)
*/
#define KWSYS_HEADER(x) KWSYS_HEADER0(KWSYS_NAMESPACE/x)
#define KWSYS_HEADER0(x) KWSYS_HEADER1(x)
#define KWSYS_HEADER1(x) <x>

#else
# error "kwsysPrivate.h included multiple times."
#endif
