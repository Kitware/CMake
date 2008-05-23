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

#ifndef QCMakeCacheView_h
#define QCMakeCacheView_h

#include "QCMake.h"
#include <QTableView>
#include <QAbstractTableModel>
#include <QItemDelegate>

class QSortFilterProxyModel;
class QCMakeCacheModel;


/// Qt view class for cache properties
class QCMakeCacheView : public QTableView
{
  Q_OBJECT
public:
  QCMakeCacheView(QWidget* p);

  QCMakeCacheModel* cacheModel() const;
  bool showAdvanced() const;

public slots:
  void setShowAdvanced(bool);
  void setSearchFilter(const QString&);

protected:
  QModelIndex moveCursor(CursorAction, Qt::KeyboardModifiers);
  void showEvent(QShowEvent* e);
  bool Init;
  QCMakeCacheModel* CacheModel;
  QSortFilterProxyModel* AdvancedFilter;
  QSortFilterProxyModel* SearchFilter;
};

/// Qt model class for cache properties
class QCMakeCacheModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  QCMakeCacheModel(QObject* parent);
  ~QCMakeCacheModel();

  enum { HelpRole = Qt::UserRole, TypeRole, AdvancedRole };

public slots:
  void setProperties(const QCMakePropertyList& props);
  void clear();
  void setEditEnabled(bool);
  bool removeRows(int row, int count, const QModelIndex& idx = QModelIndex());
  bool insertRows(int row, int num, const QModelIndex&);
  
  // insert a property at a row specifying all the information about the
  // property
  bool insertProperty(int row, QCMakeProperty::PropertyType t,
                      const QString& name, const QString& description,
                      const QVariant& value, bool advanced);

public:
  // satisfy [pure] virtuals
  int columnCount (const QModelIndex& parent) const;
  QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const;
  QModelIndex parent (const QModelIndex& index) const;
  int rowCount (const QModelIndex& parent = QModelIndex()) const;
  QVariant headerData (int section, Qt::Orientation orient, int role) const;
  Qt::ItemFlags flags (const QModelIndex& index) const;
  bool setData (const QModelIndex& index, const QVariant& value, int role);
  QModelIndex buddy (const QModelIndex& index) const;

  // get the properties
  QCMakePropertyList properties() const;
  
  // editing enabled
  bool editEnabled() const;

  int newCount() const;

protected:
  QCMakePropertyList Properties;
  int NewCount;
  bool EditEnabled;
};

/// Qt delegate class for interaction (or other customization) 
/// with cache properties
class QCMakeCacheModelDelegate : public QItemDelegate
{
  Q_OBJECT
public:
  QCMakeCacheModelDelegate(QObject* p);
  /// create our own editors for cache properties
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, 
      const QModelIndex& index ) const;
  bool editorEvent (QEvent* event, QAbstractItemModel* model, 
       const QStyleOptionViewItem& option, const QModelIndex& index);
  bool eventFilter(QObject* object, QEvent* event);
protected slots:
  void setFileDialogFlag(bool);
protected:
  bool FileDialogFlag;
};

#endif

