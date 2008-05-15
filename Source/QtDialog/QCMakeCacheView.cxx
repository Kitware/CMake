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

#include <QHBoxLayout>
#include <QHeaderView>
#include <QEvent>
#include <QStyle>
#include <QKeyEvent>
#include <QSortFilterProxyModel>

#include "QCMakeWidgets.h"

static QRegExp AdvancedRegExp[2] = { QRegExp("(false)"), QRegExp("(true|false)") };

// filter for searches
class QCMakeSearchFilter : public QSortFilterProxyModel
{
public:
  QCMakeSearchFilter(QObject* o) : QSortFilterProxyModel(o) {}
protected:
  bool filterAcceptsRow(int row, const QModelIndex& p) const
    {
    // accept row if either column matches
    QModelIndex idx0 = this->sourceModel()->index(row, 0, p);
    QModelIndex idx1 = this->sourceModel()->index(row, 1, p);
    QString str0 = this->sourceModel()->data(idx0).toString();
    QString str1 = this->sourceModel()->data(idx1).toString();

    return str0.contains(this->filterRegExp()) ||
           str1.contains(this->filterRegExp());
    }
};

QCMakeCacheView::QCMakeCacheView(QWidget* p)
  : QTableView(p), Init(false)
{
  // hook up our model and search/filter proxies
  this->CacheModel = new QCMakeCacheModel(this);
  this->AdvancedFilter = new QSortFilterProxyModel(this);
  this->AdvancedFilter->setSourceModel(this->CacheModel);
  this->AdvancedFilter->setFilterRole(QCMakeCacheModel::AdvancedRole);
  this->AdvancedFilter->setFilterRegExp(AdvancedRegExp[0]);
  this->AdvancedFilter->setDynamicSortFilter(true);
  this->SearchFilter = new QCMakeSearchFilter(this);
  this->SearchFilter->setSourceModel(this->AdvancedFilter);
  this->SearchFilter->setFilterCaseSensitivity(Qt::CaseInsensitive);
  this->SearchFilter->setDynamicSortFilter(true);
  this->setModel(this->SearchFilter);

  // our delegate for creating our editors
  QCMakeCacheModelDelegate* delegate = new QCMakeCacheModelDelegate(this);
  this->setItemDelegate(delegate);
  
  this->setEditTriggers(QAbstractItemView::DoubleClicked |
                        QAbstractItemView::SelectedClicked |
                        QAbstractItemView::EditKeyPressed |
                        QAbstractItemView::AnyKeyPressed);

  // tab, backtab doesn't step through items
  this->setTabKeyNavigation(false);

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
  return this->CacheModel;
}
 
QModelIndex QCMakeCacheView::moveCursor(CursorAction act, 
  Qt::KeyboardModifiers mod)
{
  // want home/end to go to begin/end of rows, not columns
  if(act == MoveHome)
    {
    return this->model()->index(0, 1);
    }
  else if(act == MoveEnd)
    {
    return this->model()->index(this->model()->rowCount()-1, 1);
    }
  return QTableView::moveCursor(act, mod);
}
  
void QCMakeCacheView::setShowAdvanced(bool s)
{
  this->AdvancedFilter->setFilterRegExp(
    s ? AdvancedRegExp[1] : AdvancedRegExp[0]);
}

bool QCMakeCacheView::showAdvanced() const
{
  return this->AdvancedFilter->filterRegExp() == AdvancedRegExp[1];
}

void QCMakeCacheView::setSearchFilter(const QString& s)
{
  this->SearchFilter->setFilterFixedString(s);
}

QCMakeCacheModel::QCMakeCacheModel(QObject* p)
  : QAbstractTableModel(p),
    NewCount(0), EditEnabled(true)
{
}

QCMakeCacheModel::~QCMakeCacheModel()
{
}

static uint qHash(const QCMakeProperty& p)
{
  return qHash(p.Key);
}

void QCMakeCacheModel::clear()
{
  this->setProperties(QCMakePropertyList());
}

void QCMakeCacheModel::setProperties(const QCMakePropertyList& props)
{
  QSet<QCMakeProperty> newProps = props.toSet();
  QSet<QCMakeProperty> newProps2 = props.toSet();
  QSet<QCMakeProperty> oldProps = this->Properties.toSet();
  
  oldProps.intersect(newProps);
  newProps.subtract(oldProps);
  newProps2.subtract(newProps);

  this->NewCount = newProps.count();
  this->Properties.clear();

  this->Properties = newProps.toList();
  qSort(this->Properties);
  QCMakePropertyList tmp = newProps2.toList();
  qSort(tmp);
  this->Properties += tmp;
  
  this->reset();
}
  
QCMakePropertyList QCMakeCacheModel::properties() const
{
  return this->Properties;
}
  
bool QCMakeCacheModel::insertProperty(int row, QCMakeProperty::PropertyType t,
                      const QString& name, const QString& description,
                      const QVariant& value, bool advanced)
{
  if(this->insertRows(row, 1, QModelIndex()))
    {
    QModelIndex idx1 = this->index(row, 0);
    QModelIndex idx2 = this->index(row, 1);
    
    this->setData(idx1, t, QCMakeCacheModel::TypeRole);
    this->setData(idx1, name, Qt::DisplayRole);
    this->setData(idx1, description, QCMakeCacheModel::HelpRole);
    this->setData(idx1, advanced, QCMakeCacheModel::AdvancedRole);
    if(t == QCMakeProperty::BOOL)
      {
      this->setData(idx2, value.toBool() ? Qt::Checked : Qt::Unchecked,
        Qt::CheckStateRole);
      }
    else
      {
      this->setData(idx2, value, Qt::DisplayRole);
      }
    return true;
    }
  return false;
}

void QCMakeCacheModel::setEditEnabled(bool e)
{
  this->EditEnabled = e;
}

bool QCMakeCacheModel::editEnabled() const
{
  return this->EditEnabled;
}

int QCMakeCacheModel::newCount() const
{
  return this->NewCount;
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
    if(this->Properties[idx.row()].Type != QCMakeProperty::BOOL)
      {
      return this->Properties[idx.row()].Value;
      }
    }
  else if(idx.column() == 1 && role == Qt::CheckStateRole)
    {
    if(this->Properties[idx.row()].Type == QCMakeProperty::BOOL)
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
  if(idx.column() == 1 && this->EditEnabled)
    {
    f |= Qt::ItemIsEditable;
    // booleans are editable in place
    if(this->Properties[idx.row()].Type == QCMakeProperty::BOOL)
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
    emit this->dataChanged(idx, idx);
    }
  else if(idx.column() == 1 && (role == Qt::DisplayRole || role == Qt::EditRole))
    {
    this->Properties[idx.row()].Value = value.toString();
    emit this->dataChanged(idx, idx);
    }
  else if(idx.column() == 1 && (role == Qt::CheckStateRole))
    {
    this->Properties[idx.row()].Value = value.toInt() == Qt::Checked;
    emit this->dataChanged(idx, idx);
    }
  else if(role == QCMakeCacheModel::HelpRole)
    {
    this->Properties[idx.row()].Help = value.toString();
    emit this->dataChanged(idx, idx);
    }
  else if(role == QCMakeCacheModel::TypeRole)
    {
    this->Properties[idx.row()].Type = static_cast<QCMakeProperty::PropertyType>(value.toInt());
    }
  else if(role == QCMakeCacheModel::AdvancedRole)
    {
    this->Properties[idx.row()].Advanced = value.toBool();
    }
  return false;
}

QModelIndex QCMakeCacheModel::buddy(const QModelIndex& idx) const
{
  if(idx.column() == 0)
    {
    if(this->Properties[idx.row()].Type != QCMakeProperty::BOOL)
      {
      return this->index(idx.row(), 1);
      }
    }
  return idx;
}
  
bool QCMakeCacheModel::removeRows(int row, int num, const QModelIndex&)
{
  if(row < 0 || row+num > this->Properties.count())
    {
    return false;
    }
  this->beginRemoveRows(QModelIndex(), row, row+num-1);
  for(int i=0; i<num; i++)
    {
    this->Properties.removeAt(row);
    if(this->NewCount >= row+1)
      {
      this->NewCount--;
      }
    }
  this->endRemoveRows();
  return true;
}

bool QCMakeCacheModel::insertRows(int row, int num, const QModelIndex&)
{
  if(row < 0)
    row = 0;
  if(row > this->rowCount())
    row = this->rowCount();

  this->beginInsertRows(QModelIndex(), row, row+num-1);
  for(int i=0; i<num; i++)
    {
    this->Properties.insert(row+i, QCMakeProperty());
    if(this->NewCount >= row)
      {
      this->NewCount++;
      }
    }
  this->endInsertRows();
  return true;
}

QCMakeCacheModelDelegate::QCMakeCacheModelDelegate(QObject* p)
  : QItemDelegate(p), FileDialogFlag(false)
{
}

void QCMakeCacheModelDelegate::setFileDialogFlag(bool f)
{
  this->FileDialogFlag = f;
}

QWidget* QCMakeCacheModelDelegate::createEditor(QWidget* p, 
    const QStyleOptionViewItem&, const QModelIndex& idx) const
{
  const QAbstractItemModel* model = idx.model();
  QModelIndex var = model->index(idx.row(), 0);
  QVariant type = idx.data(QCMakeCacheModel::TypeRole);
  if(type == QCMakeProperty::BOOL)
    {
    return NULL;
    }
  else if(type == QCMakeProperty::PATH)
    {
    QCMakePathEditor* editor =
      new QCMakePathEditor(p, 
      var.data(Qt::DisplayRole).toString());
    QObject::connect(editor, SIGNAL(fileDialogExists(bool)), this,
        SLOT(setFileDialogFlag(bool)));
    return editor;
    }
  else if(type == QCMakeProperty::FILEPATH)
    {
    QCMakeFilePathEditor* editor =
      new QCMakeFilePathEditor(p, 
      var.data(Qt::DisplayRole).toString());
    QObject::connect(editor, SIGNAL(fileDialogExists(bool)), this,
        SLOT(setFileDialogFlag(bool)));
    return editor;
    }

  return new QLineEdit(p);
}
  
bool QCMakeCacheModelDelegate::editorEvent(QEvent* e, QAbstractItemModel* model, 
       const QStyleOptionViewItem& option, const QModelIndex& index)
{
  Qt::ItemFlags flags = model->flags(index);
  if (!(flags & Qt::ItemIsUserCheckable) || !(option.state & QStyle::State_Enabled)
      || !(flags & Qt::ItemIsEnabled))
    {
    return false;
    }

  QVariant value = index.data(Qt::CheckStateRole);
  if (!value.isValid())
    {
    return false;
    }

  if ((e->type() == QEvent::MouseButtonRelease)
      || (e->type() == QEvent::MouseButtonDblClick))
    {
    // eat the double click events inside the check rect
    if (e->type() == QEvent::MouseButtonDblClick)
      {
      return true;
      }
    } 
  else if (e->type() == QEvent::KeyPress)
    {
    if(static_cast<QKeyEvent*>(e)->key() != Qt::Key_Space &&
       static_cast<QKeyEvent*>(e)->key() != Qt::Key_Select)
      {
      return false;
      }
    } 
  else 
    {
    return false;
    }

  Qt::CheckState state = (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked
                          ? Qt::Unchecked : Qt::Checked);
  return model->setData(index, state, Qt::CheckStateRole);
}
  
bool QCMakeCacheModelDelegate::eventFilter(QObject* object, QEvent* event)
{
  // workaround for what looks like a bug in Qt on Mac OS X
  if(event->type() == QEvent::FocusOut && this->FileDialogFlag)
  {
    return false;
  }
  return QItemDelegate::eventFilter(object, event);
}

  
