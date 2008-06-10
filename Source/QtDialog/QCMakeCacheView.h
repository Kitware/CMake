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
#include <QTreeView>
#include <QAbstractTableModel>
#include <QItemDelegate>

class QSortFilterProxyModel;
class QCMakeCacheModel;


/// Qt view class for cache properties
class QCMakeCacheView : public QTreeView
{
  Q_OBJECT
public:
  QCMakeCacheView(QWidget* p);

  // retrieve the QCMakeCacheModel storing all the pointers
  // this isn't necessarily the model one would get from model()
  QCMakeCacheModel* cacheModel() const;
  
  // get whether to show advanced entries
  bool showAdvanced() const;

public slots:
  // set whether to show advanced entries
  void setShowAdvanced(bool);
  // set the search filter string.  any property key or value not matching will
  // be filtered out
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
class QCMakeCacheModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  QCMakeCacheModel(QObject* parent);
  ~QCMakeCacheModel();

  // roles used to retrieve extra data such has help strings, types of
  // properties, and the advanced flag
  enum { HelpRole = Qt::UserRole, TypeRole, AdvancedRole };

public slots:
  // set a list of properties.  This list will be sorted and grouped according
  // to prefix.  Any property that existed already and which is found in this
  // list of properties to set will become an old property.  All others will
  // become new properties and be marked red.
  void setProperties(const QCMakePropertyList& props);

  // clear everything from the model
  void clear();

  // set flag whether the model can currently be edited.
  void setEditEnabled(bool);

  // remove properties from the model
  bool removeRows(int row, int count, const QModelIndex& idx);
  
  // insert a new property at a row specifying all the information about the
  // property
  bool insertProperty(QCMakeProperty::PropertyType t,
                      const QString& name, const QString& description,
                      const QVariant& value, bool advanced);

public:
  // satisfy [pure] virtuals
  QModelIndex index (int row, int column, const QModelIndex& parent = QModelIndex()) const;
  int columnCount (const QModelIndex& parent) const;
  QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const;
  QModelIndex parent (const QModelIndex& index) const;
  int rowCount (const QModelIndex& parent = QModelIndex()) const;
  QVariant headerData (int section, Qt::Orientation orient, int role) const;
  Qt::ItemFlags flags (const QModelIndex& index) const;
  bool setData (const QModelIndex& index, const QVariant& value, int role);
  QModelIndex buddy (const QModelIndex& index) const;
  bool hasChildren (const QModelIndex& index) const;

  // get the properties
  QCMakePropertyList properties() const;
  
  // editing enabled
  bool editEnabled() const;

  // returns if there are any new properties
  bool hasNewProperties() const;
  
protected:
  QList<QPair<QString, QCMakePropertyList> > NewProperties;
  QList<QPair<QString, QCMakePropertyList> > Properties;
  bool EditEnabled;

  // gets the internal data for a model index, if it exists
  const QCMakeProperty* propertyForIndex(const QModelIndex& idx) const;
  const QPair<QString,QCMakePropertyList>* propertyListForIndex(const QModelIndex& idx) const;
  bool isNewProperty(const QModelIndex& idx) const;

  // breaks up he property list into groups
  // where each group has the same prefix up to the first underscore
  static void breakProperties(const QSet<QCMakeProperty>& props,
                       QMap<QString, QCMakePropertyList>& result);
  
  // gets the prefix of a string up to the first _
  static QString prefix(const QString& s);

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

