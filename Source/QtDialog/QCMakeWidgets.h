/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QCMakeWidgets_h
#define QCMakeWidgets_h

#include <QLineEdit>
#include <QCompleter>
class QToolButton;

// common widgets for Qt based CMake

/// Editor widget for editing paths or file paths
class QCMakeFileEditor : public QLineEdit
{
  Q_OBJECT
public:
  QCMakeFileEditor(QWidget* p, const QString& var);
protected slots:
  virtual void chooseFile() = 0;
signals:
  void fileDialogExists(bool);
protected:
  void resizeEvent(QResizeEvent* e);
  QToolButton* ToolButton;
  QString Variable;
};

/// editor widget for editing files
class QCMakePathEditor : public QCMakeFileEditor
{
  Q_OBJECT
public:
  QCMakePathEditor(QWidget* p = NULL, const QString& var = QString());
  void chooseFile();
};

/// editor widget for editing paths
class QCMakeFilePathEditor : public QCMakeFileEditor
{
  Q_OBJECT
public:
  QCMakeFilePathEditor(QWidget* p = NULL, const QString& var = QString());
  void chooseFile();
};

/// completer class that returns native cmake paths
class QCMakeFileCompleter : public QCompleter
{
  Q_OBJECT
public:
  QCMakeFileCompleter(QObject* o, bool dirs);
  virtual QString pathFromIndex(const QModelIndex& idx) const;
};

#endif

