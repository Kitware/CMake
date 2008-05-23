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

#include "QCMakeWidgets.h"

#include <QDirModel>
#include <QFileInfo>
#include <QFileDialog>
#include <QToolButton>
#include <QResizeEvent>

QCMakeFileEditor::QCMakeFileEditor(QWidget* p, const QString& var)
  : QLineEdit(p), Variable(var)
{
  this->ToolButton = new QToolButton(this);
  this->ToolButton->setText("...");
  this->ToolButton->setCursor(QCursor(Qt::ArrowCursor));
  QObject::connect(this->ToolButton, SIGNAL(clicked(bool)),
                   this, SLOT(chooseFile()));
}

QCMakeFilePathEditor::QCMakeFilePathEditor(QWidget* p, const QString& var)
 : QCMakeFileEditor(p, var)
{
  this->setCompleter(new QCMakeFileCompleter(this, false));
}

QCMakePathEditor::QCMakePathEditor(QWidget* p, const QString& var)
 : QCMakeFileEditor(p, var)
{
  this->setCompleter(new QCMakeFileCompleter(this, true));
}

void QCMakeFileEditor::resizeEvent(QResizeEvent* e)
{
  // make the tool button fit on the right side
  int h = e->size().height();
  // move the line edit to make room for the tool button
  this->setContentsMargins(0, 0, h, 0);
  // put the tool button in its place
  this->ToolButton->resize(h, h);
  this->ToolButton->move(this->width() - h, 0);
}

void QCMakeFilePathEditor::chooseFile()
{
  // choose a file and set it
  QString path;
  QFileInfo info(this->text());
  QString title;
  if(this->Variable.isEmpty())
    {
    title = tr("Select File");
    }
  else
    {
    title = tr("Select File for %1");
    title = title.arg(this->Variable);
    }
  this->fileDialogExists(true);
  path = QFileDialog::getOpenFileName(this, title, info.absolutePath());
  this->fileDialogExists(false);
  
  if(!path.isEmpty())
    {
    this->setText(QDir::fromNativeSeparators(path));
    }
}

void QCMakePathEditor::chooseFile()
{
  // choose a file and set it
  QString path;
  QString title;
  if(this->Variable.isEmpty())
    {
    title = tr("Select Path");
    }
  else
    {
    title = tr("Select Path for %1");
    title = title.arg(this->Variable);
    }
  this->fileDialogExists(true);
  path = QFileDialog::getExistingDirectory(this, title, this->text());
  this->fileDialogExists(false);
  if(!path.isEmpty())
    {
    this->setText(QDir::fromNativeSeparators(path));
    }
}

QCMakeFileCompleter::QCMakeFileCompleter(QObject* o, bool dirs)
  : QCompleter(o)
{
  QDirModel* model = new QDirModel(this);
  if(dirs)
    {
    model->setFilter(QDir::AllDirs | QDir::Drives | QDir::NoDotAndDotDot);
    }
  this->setModel(model);
}

QString QCMakeFileCompleter::pathFromIndex(const QModelIndex& idx) const
{
  return QDir::fromNativeSeparators(QCompleter::pathFromIndex(idx));
}

