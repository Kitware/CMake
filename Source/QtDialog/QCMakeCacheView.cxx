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
#include <QFileInfo>
#include <QStyle>
#include <QKeyEvent>

QCMakeCacheView::QCMakeCacheView(QWidget* p)
  : QTableView(p), Init(false)
{
  // hook up our model
  QCMakeCacheModel* m = new QCMakeCacheModel(this);
  this->setModel(m);

  // our delegate for creating our editors
  QCMakeCacheModelDelegate* delegate = new QCMakeCacheModelDelegate(this);
  this->setItemDelegate(delegate);

  // set up headers and sizes
  int h = 0;
  QFontMetrics met(this->font());
  h = qMax(met.height(), this->style()->pixelMetric(QStyle::PM_IndicatorHeight));
  this->verticalHeader()->setDefaultSectionSize(h + 4);
  this->horizontalHeader()->setStretchLastSection(true);
  this->verticalHeader()->hide();
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
  : QAbstractTableModel(p), NewCount(0), IsDirty(false)
{
}

QCMakeCacheModel::~QCMakeCacheModel()
{
}

bool QCMakeCacheModel::isDirty() const
{
  return this->IsDirty;
}

static uint qHash(const QCMakeCacheProperty& p)
{
  return qHash(p.Key);
}

void QCMakeCacheModel::clear()
{
  this->setProperties(QCMakeCachePropertyList());
}

void QCMakeCacheModel::setProperties(const QCMakeCachePropertyList& props)
{
  QSet<QCMakeCacheProperty> newProps = props.toSet();
  QSet<QCMakeCacheProperty> oldProps = this->Properties.toSet();
  
  oldProps.intersect(newProps);
  newProps.subtract(oldProps);

  this->NewCount = newProps.count();
  this->Properties.clear();

  this->Properties = newProps.toList();
  qSort(this->Properties);
  QCMakeCachePropertyList tmp = oldProps.toList();
  qSort(tmp);
  this->Properties += tmp;
  
  this->reset();
  this->IsDirty = false;
}
  
QCMakeCachePropertyList QCMakeCacheModel::properties() const
{
  return this->Properties;
}

int QCMakeCacheModel::columnCount (const QModelIndex& /*p*/ ) const
{
  return 2;
}

QVariant QCMakeCacheModel::data (const QModelIndex& idx, int role) const
{
  if(idx.column() == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
    {
    return this->Properties[idx.row()].Key;
    }
  else if(idx.column() == 0 && role == Qt::ToolTipRole)
    {
    return this->data(idx, Qt::DisplayRole).toString() + "\n" +
           this->data(idx, QCMakeCacheModel::HelpRole).toString();
    }
  else if(idx.column() == 1 && (role == Qt::DisplayRole || role == Qt::EditRole))
    {
    if(this->Properties[idx.row()].Type != QCMakeCacheProperty::BOOL)
      {
      return this->Properties[idx.row()].Value;
      }
    }
  else if(idx.column() == 1 && role == Qt::CheckStateRole)
    {
    if(this->Properties[idx.row()].Type == QCMakeCacheProperty::BOOL)
      {
      return this->Properties[idx.row()].Value.toBool() ? Qt::Checked : Qt::Unchecked;
      }
    }
  else if(role == QCMakeCacheModel::HelpRole)
    {
    return this->Properties[idx.row()].Help;
    }
  else if(role == QCMakeCacheModel::TypeRole)
    {
    return this->Properties[idx.row()].Type;
    }
  else if(role == QCMakeCacheModel::AdvancedRole)
    {
    return this->Properties[idx.row()].Advanced;
    }
  else if(role == Qt::BackgroundRole && idx.row()+1 <= this->NewCount)
    {
    return QBrush(QColor(255,100,100));
    }
  return QVariant();
}

QModelIndex QCMakeCacheModel::parent (const QModelIndex& /*idx*/) const
{
  return QModelIndex();
}

int QCMakeCacheModel::rowCount (const QModelIndex& p) const
{
  if(p.isValid())
    {
    return 0;
    }
  return this->Properties.count();
}

QVariant QCMakeCacheModel::headerData (int section, Qt::Orientation orient, int role) const
{
  // return header labels
  if(role == Qt::DisplayRole && orient == Qt::Horizontal)
    {
    return section == 0 ? "Name" : "Value";
    }
  return QVariant();
}
  
Qt::ItemFlags QCMakeCacheModel::flags (const QModelIndex& idx) const
{
  Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  // all column 1's are editable
  if(idx.column() == 1)
    {
    f |= Qt::ItemIsEditable;
    // booleans are editable in place
    if(this->Properties[idx.row()].Type == QCMakeCacheProperty::BOOL)
      {
      f |= Qt::ItemIsUserCheckable;
      }
    }
  return f;
}


bool QCMakeCacheModel::setData (const QModelIndex& idx, const QVariant& value, int role)
{
  if(idx.column() == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
    {
    this->Properties[idx.row()].Key = value.toString();
    this->IsDirty = true;
    emit this->dataChanged(idx, idx);
    }
  else if(idx.column() == 1 && (role == Qt::DisplayRole || role == Qt::EditRole))
    {
    this->Properties[idx.row()].Value = value.toString();
    this->IsDirty = true;
    emit this->dataChanged(idx, idx);
    }
  else if(idx.column() == 1 && (role == Qt::CheckStateRole))
    {
    this->Properties[idx.row()].Value = value.toInt() == Qt::Checked;
    this->IsDirty = true;
    emit this->dataChanged(idx, idx);
    }
  return false;
}



QCMakeCacheModelDelegate::QCMakeCacheModelDelegate(QObject* p)
  : QItemDelegate(p)
{
}

QWidget* QCMakeCacheModelDelegate::createEditor(QWidget* p, 
    const QStyleOptionViewItem&, const QModelIndex& idx) const
{
  QVariant type = idx.data(QCMakeCacheModel::TypeRole);
  if(type == QCMakeCacheProperty::BOOL)
    {
    return NULL;
    }
  else if(type == QCMakeCacheProperty::PATH)
    {
    return new QCMakeCachePathEditor(idx.data().toString(), false, p);
    }
  else if(type == QCMakeCacheProperty::FILEPATH)
    {
    return new QCMakeCachePathEditor(idx.data().toString(), true, p);
    }

  return new QLineEdit(p);
}
  
QCMakeCachePathEditor::QCMakeCachePathEditor(const QString& file, bool fp,
                                             QWidget* p)
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
  this->LineEdit.selectAll();
  tb->setFocusProxy(&this->LineEdit);
  this->setFocusProxy(&this->LineEdit);
}

void QCMakeCachePathEditor::chooseFile()
{
  QString path;
  if(this->IsFilePath)
    {
    QFileInfo info(this->value());
    path = QFileDialog::getOpenFileName(this, tr("Select File"), 
        info.absolutePath());
    }
  else
    {
    path = QFileDialog::getExistingDirectory(this, tr("Select Path"), 
        this->value());
    }
  if(!path.isEmpty())
    {
    this->LineEdit.setText(path);
    }
}


