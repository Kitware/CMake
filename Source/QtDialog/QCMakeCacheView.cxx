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
#include <QMenu>
#include <QDialog>
#include <QLabel>
#include <QDialogButtonBox>

static QRegExp AdvancedRegExp[2] = { QRegExp("(false)"), QRegExp("(true|false)") };

QCMakeCacheView::QCMakeCacheView(QWidget* p)
  : QTableView(p), Init(false)
{
  // hook up our model and search/filter proxies
  this->CacheModel = new QCMakeCacheModel(this);
  this->AdvancedFilter = new QSortFilterProxyModel(this);
  this->AdvancedFilter->setSourceModel(this->CacheModel);
  this->AdvancedFilter->setFilterRole(QCMakeCacheModel::AdvancedRole);
  this->AdvancedFilter->setFilterRegExp(AdvancedRegExp[0]);
  this->SearchFilter = new QSortFilterProxyModel(this);
  this->SearchFilter->setSourceModel(this->AdvancedFilter);
  this->SearchFilter->setFilterCaseSensitivity(Qt::CaseInsensitive);
  this->SearchFilter->setFilterKeyColumn(-1); // all columns
  this->setModel(this->SearchFilter);

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
  return this->CacheModel;
}
  
void QCMakeCacheView::contextMenuEvent(QContextMenuEvent* /*e*/)
{
  QList<QModelIndex> idxs = this->selectionModel()->selectedRows();

  if(idxs.count())
    {
    QMenu* menu = new QMenu(this);
    QAction* d = NULL;
    QAction* i = NULL;
    if(this->cacheModel()->editEnabled())
      {
      QString t = idxs.count() > 1 ? tr("Delete Cache Entries") : 
                                     tr("Delete Cache Entry");
      d = menu->addAction(t);
      t = idxs.count() > 1 ? tr("Ignore Cache Entries") : 
                             tr("Ignore Cache Entry");
      i = menu->addAction(t);
      }
    QAction* h = menu->addAction(tr("Help For Cache Entry"));
    QAction* which = menu->exec(QCursor::pos());
    if(!which)
      {
      return;
      }
    
    if(which == h)
      {
      QModelIndex idx = this->selectionModel()->currentIndex();
      idx = this->SearchFilter->mapToSource(idx);
      idx = this->AdvancedFilter->mapToSource(idx);
      idx = this->cacheModel()->index(idx.row(), 0);
      QString msg = this->cacheModel()->data(idx, Qt::DisplayRole).toString() +
                    "\n\n" +
             this->cacheModel()->data(idx, QCMakeCacheModel::HelpRole).toString();
      QDialog dialog;
      dialog.setWindowTitle(tr("CMakeSetup Help"));
      QVBoxLayout* l = new QVBoxLayout(&dialog);
      QLabel* lab = new QLabel(&dialog);
      l->addWidget(lab);
      lab->setText(msg);
      lab->setWordWrap(true);
      QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Ok,
                                                    Qt::Horizontal, &dialog);
      QObject::connect(btns, SIGNAL(accepted()), &dialog, SLOT(accept()));
      l->addWidget(btns);
      dialog.exec();
      }
    else
      {
      QList<QPersistentModelIndex> pidxs;
      foreach(QModelIndex i, idxs)
        {
        i = this->SearchFilter->mapToSource(i);
        i = this->AdvancedFilter->mapToSource(i);
        pidxs.append(i);
        }
      if(which == d)
        {
        foreach(QPersistentModelIndex j, pidxs)
          {
          this->cacheModel()->removeRows(j.row(), 1);
          }
        }
      else if(which == i)
        {
        foreach(QPersistentModelIndex j, pidxs)
          {
          j = this->cacheModel()->index(j.row(), 1);
          this->cacheModel()->setData(j, "IGNORE", Qt::DisplayRole);
          }
        }
      }
    }
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
    NewCount(0), ModifiedValues(false), EditEnabled(true)
{
}

QCMakeCacheModel::~QCMakeCacheModel()
{
}

bool QCMakeCacheModel::modifiedValues() const
{
  return this->ModifiedValues;
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
  
  this->ModifiedValues = NewCount != 0;
  this->reset();
}
  
QCMakeCachePropertyList QCMakeCacheModel::properties() const
{
  return this->Properties;
}

void QCMakeCacheModel::setEditEnabled(bool e)
{
  this->EditEnabled = e;
}

bool QCMakeCacheModel::editEnabled() const
{
  return this->EditEnabled;
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
  if(idx.column() == 1 && this->EditEnabled)
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
    this->ModifiedValues = true;
    emit this->dataChanged(idx, idx);
    }
  else if(idx.column() == 1 && (role == Qt::DisplayRole || role == Qt::EditRole))
    {
    this->Properties[idx.row()].Value = value.toString();
    this->ModifiedValues = true;
    emit this->dataChanged(idx, idx);
    }
  else if(idx.column() == 1 && (role == Qt::CheckStateRole))
    {
    this->Properties[idx.row()].Value = value.toInt() == Qt::Checked;
    this->ModifiedValues = true;
    emit this->dataChanged(idx, idx);
    }
  return false;
}

QModelIndex QCMakeCacheModel::buddy ( const QModelIndex& idx ) const
{
  if(idx.column() == 0)
    {
    return this->index(idx.row(), 1);
    }
  return idx;
}
  
bool QCMakeCacheModel::removeRows(int row, int, const QModelIndex&)
{
  if(row < 0 || row >= this->Properties.count())
    {
    return false;
    }
  this->beginRemoveRows(QModelIndex(), row, row);
  this->Properties.removeAt(row);
  if(this->NewCount >= row+1)
    {
    this->NewCount--;
    }
  this->endRemoveRows();
  return true;
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
    return new QCMakeCachePathEditor(false, p);
    }
  else if(type == QCMakeCacheProperty::FILEPATH)
    {
    return new QCMakeCachePathEditor(true, p);
    }

  return new QLineEdit(p);
}
  
QCMakeCachePathEditor::QCMakeCachePathEditor(bool fp, QWidget* p)
  : QLineEdit(p), IsFilePath(fp)
{
  // this *is* instead of has a line edit so QAbstractItemView
  // doesn't get confused with what the editor really is
  this->setContentsMargins(0, 0, 0, 0);
  this->ToolButton = new QToolButton(this);
  this->ToolButton->setText("...");
  this->ToolButton->setCursor(QCursor(Qt::ArrowCursor));
  QObject::connect(this->ToolButton, SIGNAL(clicked(bool)),
                   this, SLOT(chooseFile()));
}

void QCMakeCachePathEditor::resizeEvent(QResizeEvent* e)
{
  // make the tool button fit on the right side
  int h = e->size().height();
  this->ToolButton->resize(h, h);
  this->ToolButton->move(this->width() - h, 0);
  this->setContentsMargins(0, 0, h, 0);
}

void QCMakeCachePathEditor::chooseFile()
{
  // choose a file and set it
  QString path;
  if(this->IsFilePath)
    {
    QFileInfo info(this->text());
    path = QFileDialog::getOpenFileName(this, tr("Select File"), 
        info.absolutePath());
    }
  else
    {
    path = QFileDialog::getExistingDirectory(this, tr("Select Path"), 
        this->text());
    }
  if(!path.isEmpty())
    {
    this->setText(path);
    }
}


