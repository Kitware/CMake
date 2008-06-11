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
    QStringList strs;
    const QAbstractItemModel* m = this->sourceModel();
    QModelIndex idx = m->index(row, 0, p);

    // if there are no children, get strings for column 0 and 1
    if(!m->hasChildren(idx))
      {
      strs.append(m->data(idx).toString());
      idx = m->index(row, 1, p);
      strs.append(m->data(idx).toString());
      }
    else
      {
      // get strings for children entries to compare with
      // instead of comparing with the parent
      int num = m->rowCount(idx);
      for(int i=0; i<num; i++)
        {
        QModelIndex tmpidx = m->index(i, 0, idx);
        strs.append(m->data(tmpidx).toString());
        tmpidx = m->index(i, 1, idx);
        strs.append(m->data(tmpidx).toString());
        }
      }

    // check all strings for a match
    foreach(QString str, strs)
      {
      if(str.contains(this->filterRegExp()))
        {
        return true;
        }
      }
    
    return false;
    }
};

QCMakeCacheView::QCMakeCacheView(QWidget* p)
  : QTreeView(p), Init(false)
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
  this->header()->setDefaultSectionSize(h + 4);
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
  return QTreeView::showEvent(e);
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
  return QTreeView::moveCursor(act, mod);
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
  : QAbstractItemModel(p),
    EditEnabled(true)
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
  QSet<QCMakeProperty> newProps2 = newProps;
  QSet<QCMakeProperty> oldProps = this->properties().toSet();
  
  oldProps.intersect(newProps);
  newProps.subtract(oldProps);
  newProps2.subtract(newProps);

  this->Properties.clear();
  this->NewProperties.clear();

  QMap<QString, QCMakePropertyList> result;
  this->breakProperties(newProps, result);
  foreach(QString key, result.keys())
    {
    this->NewProperties.append(QPair<QString, QCMakePropertyList>(key, result.value(key)));
    }
  result.clear();
  this->breakProperties(newProps2, result);
  foreach(QString key, result.keys())
    {
    this->Properties.append(QPair<QString, QCMakePropertyList>(key, result.value(key)));
    }

  // tell everyone to discard whatever they knew (cached) about this model
  this->reset();
}

QString QCMakeCacheModel::prefix(const QString& s)
{
  QString prefix = s.section('_', 0, 0);
  if(prefix == s)
    {
    prefix = QString();
    }
  return prefix;
}

void QCMakeCacheModel::breakProperties(const QSet<QCMakeProperty>& props,
                     QMap<QString, QCMakePropertyList>& result)
{
  QMap<QString, QCMakePropertyList> tmp;
  // return a map of properties grouped by prefixes, and sorted
  foreach(QCMakeProperty p, props)
    {
    QString prefix = QCMakeCacheModel::prefix(p.Key);
    tmp[prefix].append(p);
    }
  // sort it and re-org any properties with only one sub item
  QCMakePropertyList reorgProps;
  QMap<QString, QCMakePropertyList>::iterator iter;
  for(iter = tmp.begin(); iter != tmp.end();)
    {
    if(iter->count() == 1)
      {
      reorgProps.append((*iter)[0]);
      iter = tmp.erase(iter);
      }
    else
      {
      qSort(*iter);
      ++iter;
      }
    }
  if(reorgProps.count())
    {
    tmp[QCMakeCacheModel::prefix("NOPREFIX")] += reorgProps;
    }
  result = tmp;
}
  
QCMakePropertyList QCMakeCacheModel::properties() const
{
  QCMakePropertyList props;
  QPair<QString, QCMakePropertyList> l;
  foreach(l, this->NewProperties)
    {
    props += l.second;
    }
  foreach(l, this->Properties)
    {
    props += l.second;
    }
  return props;
}
  
bool QCMakeCacheModel::insertProperty(QCMakeProperty::PropertyType t,
                      const QString& name, const QString& description,
                      const QVariant& value, bool advanced)
{
  QCMakeProperty prop;
  prop.Key = name;
  prop.Value = value;
  prop.Help = description;
  prop.Type = t;
  prop.Advanced = advanced;

  // find where to insert it in the new properties section
  QString prefix = this->prefix(name);
  QList<QPair<QString, QCMakePropertyList> >::iterator iter = this->NewProperties.begin();
  while(iter != this->NewProperties.end() && prefix > iter->first)
    {
    ++iter;
    }

  bool insertedParent = false;

  // insert a new parent item for this group of properties with this prefix
  // if there isn't one
  if(iter == this->NewProperties.end() || iter->first != prefix)
    {
    int row = iter - this->NewProperties.begin();
    this->beginInsertRows(QModelIndex(), row, row);
    iter = this->NewProperties.insert(iter, QPair<QString, QCMakePropertyList>(prefix, QCMakePropertyList()));
    insertedParent = true;
    }

  // find where to insert the property in the group of properties
  QCMakePropertyList::iterator jter = iter->second.begin();
  while(jter != iter->second.end() && name > jter->Key)
    {
    ++jter;
    }
  
  QModelIndex idxp = this->index(iter - this->NewProperties.begin(), 0);

  if(jter != iter->second.end() && jter->Key == name)
    {
    // replace existing item
    *jter = prop;
    QModelIndex idx1 = this->index(jter - iter->second.begin(), 0, idxp);
    QModelIndex idx2 = this->index(jter - iter->second.begin(), 1, idxp);
    this->dataChanged(idx1, idx2);
    }
  else
    {
    // add new item
    int row = jter - iter->second.begin();
    if(!insertedParent)
      {
      this->beginInsertRows(idxp, row, row);
      }
    jter = iter->second.insert(jter, prop);
    if(!insertedParent)
      {
      this->endInsertRows();
      }
    }
    
  if(insertedParent)
  {
    this->endInsertRows();
  }

  return true;
}

void QCMakeCacheModel::setEditEnabled(bool e)
{
  this->EditEnabled = e;
}

bool QCMakeCacheModel::editEnabled() const
{
  return this->EditEnabled;
}

bool QCMakeCacheModel::hasNewProperties() const
{
  return !this->NewProperties.isEmpty();
}

int QCMakeCacheModel::columnCount (const QModelIndex& /*p*/ ) const
{
  return 2;
}

bool QCMakeCacheModel::isNewProperty(const QModelIndex& idx) const
{
  if(idx.isValid() && idx.internalId())
    {
    return (idx.internalId() - 1) < this->NewProperties.count();
    }
  else if(idx.isValid())
    {
    return idx.row() < this->NewProperties.count();
    }
  return false;
}
  
const QCMakeProperty* QCMakeCacheModel::propertyForIndex(const QModelIndex& idx) const
{
  const QPair<QString, QCMakePropertyList>* l = this->propertyListForIndex(idx);
  if(l && idx.internalId())
    {
    if(idx.row() < l->second.count())
      {
      return &l->second[idx.row()];
      }
    }
  return NULL;
}

const QPair<QString, QCMakePropertyList>* QCMakeCacheModel::propertyListForIndex(const QModelIndex& idx) const
{
  int row = -1;
  if(idx.isValid() && idx.internalId() > 0)
    {
    row = idx.internalId() - 1;
    }
  else if(idx.isValid())
    {
    row = idx.row();
    }
  if(row != -1)
    {
    if(row < this->NewProperties.count())
      {
      return &this->NewProperties[row];
      }
    row -= this->NewProperties.count();
    if(row < this->Properties.count())
      {
      return &this->Properties[row];
      }
    }
  return NULL;
}

QVariant QCMakeCacheModel::data (const QModelIndex& idx, int role) const
{
  const QPair<QString, QCMakePropertyList>* l = this->propertyListForIndex(idx);
  const QCMakeProperty* p = propertyForIndex(idx);
  if(l && !p)
    {
    if(idx.column() == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
      {
      if(l->first == QString())
        {
        return tr("Ungrouped Properties");
        }
      return l->first;
      }
    else if(role == QCMakeCacheModel::AdvancedRole)
      {
      // return true if all subitems are advanced
      foreach(QCMakeProperty p, l->second)
        {
        if(!p.Advanced)
          {
          return false;
          }
        }
      return true;
      }
    }
  if(l && p)
    {
    if(idx.column() == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
      {
      return p->Key;
      }
    else if(idx.column() == 0 && role == Qt::ToolTipRole)
      {
      return p->Key + "\n" + p->Help;
      }
    else if(idx.column() == 1 && (role == Qt::DisplayRole || role == Qt::EditRole))
      {
      if(p->Type != QCMakeProperty::BOOL)
        {
        return p->Value;
        }
      }
    else if(idx.column() == 1 && role == Qt::CheckStateRole)
      {
      if(p->Type == QCMakeProperty::BOOL)
        {
        return p->Value.toBool() ? Qt::Checked : Qt::Unchecked;
        }
      }
    else if(role == QCMakeCacheModel::HelpRole)
      {
      return p->Help;
      }
    else if(role == QCMakeCacheModel::TypeRole)
      {
      return p->Type;
      }
    else if(role == QCMakeCacheModel::AdvancedRole)
      {
      return p->Advanced;
      }
    }
  
  if(role == Qt::BackgroundRole && this->isNewProperty(idx))
    {
    return QBrush(QColor(255,100,100));
    }

  return QVariant();
}

QModelIndex QCMakeCacheModel::parent (const QModelIndex& idx) const
{
  if(idx.isValid() && idx.internalId())
    {
    return this->createIndex(idx.internalId()-1, 0);
    }
  return QModelIndex();
}

QModelIndex QCMakeCacheModel::index (int row, int column, const QModelIndex& idx) const
{
  if(!idx.isValid() && row < this->rowCount(idx) && column < this->columnCount(idx))
    {
    // index at root level
    return this->createIndex(row, column);
    }
  else if(idx.isValid() && !idx.internalId() && row < this->rowCount(idx) && column < this->columnCount(idx))
    {
    // index at sub-item
    return this->createIndex(row, column, idx.row()+1);
    }
  return QModelIndex();
}

bool QCMakeCacheModel::hasChildren (const QModelIndex& idx) const
{
  if(idx.isValid() && idx.internalId())
    {
    return false;
    }
  return true;
}

int QCMakeCacheModel::rowCount (const QModelIndex& idx) const
{
  if(!idx.isValid())
    {
    int num = this->Properties.count() + this->NewProperties.count();
    return num;
    }
  
  const QPair<QString, QCMakePropertyList>* l = this->propertyListForIndex(idx);
  const QCMakeProperty* p = this->propertyForIndex(idx);

  if(l && !p)
    {
    return l->second.count();
    }
  
  return 0;
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
  const QCMakeProperty* p = this->propertyForIndex(idx);

  // all column 1's are editable
  if(p && idx.column() == 1 && this->EditEnabled)
    {
    f |= Qt::ItemIsEditable;
    // booleans are editable in place
    if(p->Type == QCMakeProperty::BOOL)
      {
      f |= Qt::ItemIsUserCheckable;
      }
    }
  return f;
}


bool QCMakeCacheModel::setData (const QModelIndex& idx, const QVariant& value, int role)
{
  QCMakeProperty* p = const_cast<QCMakeProperty*>(this->propertyForIndex(idx));
  if(p)
    {
    if(idx.column() == 1 && (role == Qt::DisplayRole || role == Qt::EditRole))
      {
      p->Value = value.toString();
      emit this->dataChanged(idx, idx);
      return true;
      }
    else if(idx.column() == 1 && (role == Qt::CheckStateRole))
      {
      p->Value = value.toInt() == Qt::Checked;
      emit this->dataChanged(idx, idx);
      return true;
      }
    }
  return false;
}

QModelIndex QCMakeCacheModel::buddy(const QModelIndex& idx) const
{
  const QCMakeProperty* p = this->propertyForIndex(idx);
  if(p && idx.column() == 0)
    {
    if(p->Type != QCMakeProperty::BOOL)
      {
      return this->index(idx.row(), 1, idx.parent());
      }
    }
  return idx;
}
  
bool QCMakeCacheModel::removeRows(int row, int count, const QModelIndex& p)
{
  // remove parent items
  if(!p.isValid())
    {
    this->beginRemoveRows(p, row, row-1+count);
    while(count != 0 && row < this->NewProperties.count())
      {
      this->NewProperties.removeAt(row);
      count--;
      }
    row -= this->NewProperties.count();
    while(count != 0 && row < this->Properties.count())
      {
      this->Properties.removeAt(row);
      count--;
      }
    this->endRemoveRows();
    return true;
    }

  // get the parent item containing the item to remove
  QPair<QString, QCMakePropertyList>* l = 
    const_cast<QPair<QString, QCMakePropertyList>*>(this->propertyListForIndex(p));
  if(!l || l->second.count() < row)
    {
    return false;
    }

  // all items under parent item are being removed, remove the parent item too
  if(l->second.count() == count && row == 0)
    {
    return this->removeRows(p.row(), 1, QModelIndex());
    }

  // remove the sub items
  if(l->second.count() >= count + row)
    {
    this->beginRemoveRows(p, row, row-1+count);
    l->second.erase(l->second.begin()+row, l->second.begin()+row+count);
    this->endRemoveRows();
    return true;
    }

  return false;
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
  // where it doesn't create a QWidget wrapper for the native file dialog
  // so the Qt library ends up assuming the focus was lost to something else
  if(event->type() == QEvent::FocusOut && this->FileDialogFlag)
    {
    return false;
    }
  return QItemDelegate::eventFilter(object, event);
}

  
