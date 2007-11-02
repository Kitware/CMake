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

#include "QCMakeCacheView.h"

#include <QToolButton>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QEvent>

QCMakeCacheView::QCMakeCacheView(QWidget* p)
  : QTableView(p)
{
  QCMakeCacheModel* m = new QCMakeCacheModel(this);
  this->setModel(m);
  this->horizontalHeader()->setStretchLastSection(true);
  this->verticalHeader()->hide();

  QCMakeCacheModelDelegate* delegate = new QCMakeCacheModelDelegate(this);
  this->setItemDelegate(delegate);
}

bool QCMakeCacheView::event(QEvent* e)
{
  if(e->type() == QEvent::Polish)
    {
    // initialize the table view column size
    int colWidth = this->columnWidth(0) + this->columnWidth(1);
    this->setColumnWidth(0, colWidth/2);
    this->setColumnWidth(1, colWidth/2);
    }
  return QTableView::event(e);
}
  
QCMakeCacheModel* QCMakeCacheView::cacheModel() const
{
  return qobject_cast<QCMakeCacheModel*>(this->model());
}

QCMakeCacheModel::QCMakeCacheModel(QObject* p)
  : QAbstractTableModel(p)
{
}

QCMakeCacheModel::~QCMakeCacheModel()
{
}

void QCMakeCacheModel::setProperties(const QCMakeCachePropertyList& props)
{
  this->Properties = props;
  this->reset();
}
  
QCMakeCachePropertyList QCMakeCacheModel::properties() const
{
  return this->Properties;
}

int QCMakeCacheModel::columnCount ( const QModelIndex & parent ) const
{
  return 2;
}

QVariant QCMakeCacheModel::data ( const QModelIndex & index, int role ) const
{
  if(index.column() == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
  {
    return this->Properties[index.row()].Key;
  }
  else if(index.column() == 1 && (role == Qt::DisplayRole || role == Qt::EditRole))
  {
    return this->Properties[index.row()].Value;
  }
  else if(role == QCMakeCacheModel::HelpRole)
  {
    return this->Properties[index.row()].Help;
  }
  else if(role == QCMakeCacheModel::TypeRole)
  {
    return this->Properties[index.row()].Type;
  }
  else if(role == QCMakeCacheModel::AdvancedRole)
  {
    return this->Properties[index.row()].Advanced;
  }
  return QVariant();
}

QModelIndex QCMakeCacheModel::parent ( const QModelIndex & index ) const
{
  return QModelIndex();
}

int QCMakeCacheModel::rowCount ( const QModelIndex & parent ) const
{
  if(parent.isValid())
    return 0;
  return this->Properties.count();
}

QVariant QCMakeCacheModel::headerData ( int section, Qt::Orientation orient, int role ) const
{
  if(role == Qt::DisplayRole && orient == Qt::Horizontal)
  {
    return section == 0 ? "Name" : "Value";
  }
  return QVariant();
}
  
Qt::ItemFlags QCMakeCacheModel::flags ( const QModelIndex& index ) const
{
  if(index.column() == 1)
  {
    return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
  }
  return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}


bool QCMakeCacheModel::setData ( const QModelIndex & index, const QVariant& value, int role )
{
  if(index.column() == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
  {
    this->Properties[index.row()].Key = value.toString();
  }
  else if(index.column() == 1 && (role == Qt::DisplayRole || role == Qt::EditRole))
  {
    this->Properties[index.row()].Value = value.toString();
  }
  return false;
}



QCMakeCacheModelDelegate::QCMakeCacheModelDelegate(QObject* p)
  : QItemDelegate(p)
{
}

QWidget* QCMakeCacheModelDelegate::createEditor(QWidget* parent, 
    const QStyleOptionViewItem&, const QModelIndex& index) const
{
  QVariant type = index.data(QCMakeCacheModel::TypeRole);
  if(type == QCMakeCacheProperty::BOOL)
  {
    return new QCMakeCacheBoolEditor(index.data().toString(), parent);
  }
  else if(type == QCMakeCacheProperty::PATH)
  {
    return new QCMakeCachePathEditor(index.data().toString(), parent);
  }
  else if(type == QCMakeCacheProperty::FILEPATH)
  {
  }

  return new QLineEdit(parent);
}


QCMakeCachePathEditor::QCMakeCachePathEditor(const QString& file, QWidget* p)
  : QWidget(p), LineEdit(this)
{
  QHBoxLayout* l = new QHBoxLayout(this);
  l->setMargin(0);
  l->setSpacing(0);
  l->addWidget(&this->LineEdit);
  QToolButton* tb = new QToolButton(this);
  tb->setText("...");
  l->addWidget(tb);
  QObject::connect(tb, SIGNAL(clicked(bool)),
                   this, SLOT(chooseFile()));
  this->LineEdit.setText(file);
  tb->setFocusProxy(&this->LineEdit);
  this->setFocusProxy(&this->LineEdit);
}

void QCMakeCachePathEditor::chooseFile()
{
  QString path = QFileDialog::getExistingDirectory(this, "TODO", this->value());
  if(!path.isEmpty())
  {
    this->LineEdit.setText(path);
  }
}

QString QCMakeCachePathEditor::value() const
{
  return this->LineEdit.text();
}



