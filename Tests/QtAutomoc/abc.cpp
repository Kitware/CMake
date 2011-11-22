/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2011 Kitware, Inc.
  Copyright 2011 Alexander Neundorf (neundorf@kde.org)

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/


#include "abc.h"

#include <stdio.h>

class PrintAbc : public QObject
{
  Q_OBJECT
  public:
    PrintAbc():QObject() {}
  public slots:
    void print() const { printf("abc\n"); }
};

Abc::Abc()
:QObject()
{
}


void Abc::doAbc()
{
  PrintAbc pa;
  pa.print();
}

#include "abc.moc"
#include "moc_abc.cpp"
