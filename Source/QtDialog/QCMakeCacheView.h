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
#include <QCheckBox>
#include <QLineEdit>
#include <QItemDelegate>
#include <QSortFilterProxyModel>

class QCMakeCacheModel;
class QToolButton;


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
  void contextMenuEvent(QContextMenuEvent* e);
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
  void setProperties(const QCMakeCachePropertyList& props);
  void clear();
  void setEditEnabled(bool);
  bool removeRows(int row, int count, const QModelIndex& idx = QModelIndex());

public:
  // satisfy [pure] virtuals
  int columnCount ( const QModelIndex & parent ) const;
  QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole) const;
  QModelIndex parent ( const QModelIndex & index ) const;
  int rowCount ( const QModelIndex & parent ) const;
  QVariant headerData ( int section, Qt::Orientation orient, int role ) const;
  Qt::ItemFlags flags ( const QModelIndex& index ) const;
  bool setData ( const QModelIndex& index, const QVariant& value, int role );
  QModelIndex buddy ( const QModelIndex& index ) const;

  // get the properties
  QCMakeCachePropertyList properties() const;
  
  // editing enabled
  bool editEnabled() const;

  int newCount() const;

protected:
  QCMakeCachePropertyList Properties;
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
};

/// Editor widget for editing paths or file paths
class QCMakeCachePathEditor : public QLineEdit
{
  Q_OBJECT
public:
  QCMakeCachePathEditor(bool isFilePath, QWidget* p);
protected slots:
  void chooseFile();
protected:
  void resizeEvent(QResizeEvent* e);
  bool IsFilePath;
  QToolButton* ToolButton;
};

#endif

