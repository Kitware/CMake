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
  : QTableView(p), Init(false)
{
  QCMakeCacheModel* m = new QCMakeCacheModel(this);
  this->setModel(m);
  this->horizontalHeader()->setStretchLastSection(true);
  this->verticalHeader()->hide();

  QCMakeCacheModelDelegate* delegate = new QCMakeCacheModelDelegate(this);
  this->setItemDelegate(delegate);
}

void QCMakeCacheView::showEvent(QShowEvent* e)
{
  if(!this->Init)
    {
    // initialize the table view column size
    int colWidth = this->columnWidth(0) + this->columnWidth(1);
    this->setColumnWidth(0, colWidth/2);
    this->setColumnWidth(1, colWidth/2);
    this->Init = true;
    }
  return QTableView::showEvent(e);
}
  
QCMakeCacheModel* QCMakeCacheView::cacheModel() const
{
  return qobject_cast<QCMakeCacheModel*>(this->model());
}

QModelIndex QCMakeCacheView::moveCursor(CursorAction act, 
  Qt::KeyboardModifiers mod)
{
  // tab through values only (not names)
  QModelIndex current = this->currentIndex();
  if(act == MoveNext)
    {
    if(!current.isValid())
      {
      return this->model()->index(0, 1);
      }
    else if(current.column() == 0)
      {
      return this->model()->index(current.row(), 1);
      }
    else
      {
      return this->model()->index(current.row()+1, 1);
      }
    }
  else if(act == MovePrevious)
    {
    if(!current.isValid())
      {
      return this->model()->index(0, 1);
      }
    else
      {
      return this->model()->index(current.row()-1, 1);
      }
    }
  return QTableView::moveCursor(act, mod);
}

QCMakeCacheModel::QCMakeCacheModel(QObject* p)
  : QAbstractTableModel(p), IsDirty(false)
{
}

QCMakeCacheModel::~QCMakeCacheModel()
{
}

bool QCMakeCacheModel::isDirty() const
{
  return this->IsDirty;
}

void QCMakeCacheModel::setProperties(const QCMakeCachePropertyList& props)
{
  this->Properties = props;
  this->reset();
  this->IsDirty = false;
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
  else if(index.column() == 0 && role == Qt::ToolTipRole)
    {
    return this->data(index, Qt::DisplayRole).toString() + "\n" +
           this->data(index, QCMakeCacheModel::HelpRole).toString();
    }
  else if(index.column() == 1 && (role == Qt::DisplayRole || role == Qt::EditRole))
    {
    if(this->Properties[index.row()].Type != QCMakeCacheProperty::BOOL)
      {
      return this->Properties[index.row()].Value;
      }
    }
  else if(index.column() == 1 && role == Qt::CheckStateRole)
    {
    if(this->Properties[index.row()].Type == QCMakeCacheProperty::BOOL)
      {
      return this->Properties[index.row()].Value.toBool() ? Qt::Checked : Qt::Unchecked;
      }
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
    {
    return 0;
    }
  return this->Properties.count();
}

QVariant QCMakeCacheModel::headerData ( int section, Qt::Orientation orient, int role ) const
{
  // return header labels
  if(role == Qt::DisplayRole && orient == Qt::Horizontal)
    {
    return section == 0 ? "Name" : "Value";
    }
  return QVariant();
}
  
Qt::ItemFlags QCMakeCacheModel::flags ( const QModelIndex& index ) const
{
  Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  // all column 1's are editable
  if(index.column() == 1)
    {
    f |= Qt::ItemIsEditable;
    // booleans are editable in place
    if(this->Properties[index.row()].Type == QCMakeCacheProperty::BOOL)
      {
      f |= Qt::ItemIsUserCheckable;
      }
    }
  return f;
}


bool QCMakeCacheModel::setData ( const QModelIndex & index, const QVariant& value, int role )
{
  if(index.column() == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
    {
    this->Properties[index.row()].Key = value.toString();
    this->IsDirty = true;
    emit this->dataChanged(index, index);
    }
  else if(index.column() == 1 && (role == Qt::DisplayRole || role == Qt::EditRole))
    {
    this->Properties[index.row()].Value = value.toString();
    this->IsDirty = true;
    emit this->dataChanged(index, index);
    }
  else if(index.column() == 1 && (role == Qt::CheckStateRole))
    {
    this->Properties[index.row()].Value = value.toInt() == Qt::Checked;
    this->IsDirty = true;
    emit this->dataChanged(index, index);
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
    return NULL;
    }
  else if(type == QCMakeCacheProperty::PATH)
    {
    return new QCMakeCachePathEditor(index.data().toString(), false, parent);
    }
  else if(type == QCMakeCacheProperty::FILEPATH)
    {
    return new QCMakeCachePathEditor(index.data().toString(), true, parent);
    }

  return new QLineEdit(parent);
}


QCMakeCachePathEditor::QCMakeCachePathEditor(const QString& file, bool fp, QWidget* p)
  : QWidget(p), LineEdit(this), IsFilePath(fp)
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
  QString path;
  if(this->IsFilePath)
    {
    path = QFileDialog::getOpenFileName(this, "TODO");
    }
  else
    {
    path = QFileDialog::getExistingDirectory(this, "TODO", this->value());
    }
  if(!path.isEmpty())
    {
    this->LineEdit.setText(path);
    }
}


