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

#include <QTableView>
#include <QAbstractTableModel>
#include <QComboBox>
#include <QLineEdit>
#include <QItemDelegate>

#include "QCMake.h"
class QCMakeCacheModel;


/// Qt view class for cache properties
class QCMakeCacheView : public QTableView
{
  Q_OBJECT
public:
  QCMakeCacheView(QWidget* p);

  QCMakeCacheModel* cacheModel() const;

protected:
  bool event(QEvent*);
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

public:
  int columnCount ( const QModelIndex & parent ) const;
  QVariant data ( const QModelIndex & index, int role ) const;
  QModelIndex parent ( const QModelIndex & index ) const;
  int rowCount ( const QModelIndex & parent ) const;
  QVariant headerData ( int section, Qt::Orientation orient, int role ) const;
  Qt::ItemFlags flags ( const QModelIndex& index ) const;
  bool setData ( const QModelIndex& index, const QVariant& value, int role );

  QCMakeCachePropertyList properties() const;

protected:
  QCMakeCachePropertyList Properties;
};

/// Qt delegate class for interaction (or other customization) with cache properties
class QCMakeCacheModelDelegate : public QItemDelegate
{
  Q_OBJECT
public:
  QCMakeCacheModelDelegate(QObject* p);
  QWidget* createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

/// Editor widget for editing paths
class QCMakeCachePathEditor : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QString value READ value USER true)
public:
  QCMakeCachePathEditor(const QString& file, QWidget* p);
  QString value() const;
protected slots:
  void chooseFile();
protected:
  QLineEdit LineEdit;
};

/// Editor widget for editing file paths
class QCMakeCacheFilePathEditor : public QWidget
{
};

/// Editor widget for editing booleans
class QCMakeCacheBoolEditor : public QComboBox
{
  Q_OBJECT
  Q_PROPERTY(QString value READ currentText USER true)
public:
  QCMakeCacheBoolEditor(const QString& val, QWidget* p)
    : QComboBox(p)
  {
    this->addItem("ON");
    this->addItem("OFF");
    this->setCurrentIndex(val == "ON" ? 0 : 1);
  }
};

#endif

