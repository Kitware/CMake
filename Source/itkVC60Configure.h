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
/**
 * itkVC60Configure : a class that configures itk for build
 * on windows with VC60
 */
#ifndef itkVC60Configure_h
#define itkVC60Configure_h

#include "cmWindowsConfigure.h"

class itkVC60Configure : public cmWindowsConfigure
{
public:
  /** 
   * implement configure from parent
   */
  virtual void Configure();
  /**
   * create the main itk configure file
   */
  virtual void GenerateITKConfigHeader();
  /**
   * Create the vnl configure file
   */
  virtual void GenerateVNLConfigHeader();
protected:
  void CopyFileTo(const char* source,
                  const char* destdir,
                  const char* dest);
};

#endif
