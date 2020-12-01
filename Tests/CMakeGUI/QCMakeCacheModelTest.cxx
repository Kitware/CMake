/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "QCMakeCacheModelTest.h"

#include <algorithm>
#include <iostream>

#include "QCMakeCacheView.h"
#include <QtTest>

namespace {
QCMakeProperty makeProperty(
  const QString& name, const QString& value,
  QCMakeProperty::PropertyType type = QCMakeProperty::STRING,
  bool advanced = false)
{
  return QCMakeProperty{
    /*Key=*/name,
    /*Value=*/value,
    /*Strings=*/{},
    /*Help=*/"",
    /*Type=*/type,
    /*Advanced=*/advanced,
  };
}
}

void QCMakeCacheModelTest::setNewProperties()
{
  QFETCH(QCMakePropertyList, oldList);
  QFETCH(QCMakePropertyList, newList);
  QFETCH(QVector<QString>, names);
  QFETCH(QVector<QString>, values);
  QFETCH(QVector<QVariant>, background);

  QCMakeCacheModel model;
  model.setViewType(QCMakeCacheModel::FlatView);
  model.setProperties(oldList);
  model.setProperties(newList);

  auto rows = model.rowCount();
  QVector<QString> actualNames(rows);
  QVector<QString> actualValues(rows);
  QVector<QVariant> actualBackground1(rows);
  QVector<QVariant> actualBackground2(rows);
  for (int i = 0; i < rows; ++i) {
    auto idx1 = model.index(i, 0);
    auto idx2 = model.index(i, 1);

    auto name = model.data(idx1, Qt::DisplayRole);
    QVERIFY(name.canConvert<QString>());
    actualNames[i] = name.value<QString>();

    auto value = model.data(idx2, Qt::DisplayRole);
    QVERIFY(name.canConvert<QString>());
    actualValues[i] = value.value<QString>();

    actualBackground1[i] = model.data(idx1, Qt::BackgroundRole);
    actualBackground2[i] = model.data(idx2, Qt::BackgroundRole);
  }

  QCOMPARE(actualNames, names);
  QCOMPARE(actualValues, values);
  QCOMPARE(actualBackground1, background);
  QCOMPARE(actualBackground2, background);
}

void QCMakeCacheModelTest::setNewProperties_data()
{
  QTest::addColumn<QCMakePropertyList>("oldList");
  QTest::addColumn<QCMakePropertyList>("newList");
  QTest::addColumn<QVector<QString>>("names");
  QTest::addColumn<QVector<QString>>("values");
  QTest::addColumn<QVector<QVariant>>("background");

  QTest::newRow("empty") << QCMakePropertyList{} << QCMakePropertyList{}
                         << QVector<QString>{} << QVector<QString>{}
                         << QVector<QVariant>{};
  QTest::newRow("noNew") << QCMakePropertyList{ makeProperty("VARIABLE_1",
                                                             "Value 1") }
                         << QCMakePropertyList{} << QVector<QString>{}
                         << QVector<QString>{} << QVector<QVariant>{};
  QTest::newRow("allNew")
    << QCMakePropertyList{}
    << QCMakePropertyList{ makeProperty("VARIABLE_1", "Value 1") }
    << QVector<QString>{ "VARIABLE_1" } << QVector<QString>{ "Value 1" }
    << QVector<QVariant>{ QBrush{ QColor{ 255, 100, 100 } } };
  QTest::newRow("mixed")
    << QCMakePropertyList{ makeProperty("VARIABLE_1", "Value 1") }
    << QCMakePropertyList{ makeProperty("VARIABLE_2", "Value 2") }
    << QVector<QString>{ "VARIABLE_2" } << QVector<QString>{ "Value 2" }
    << QVector<QVariant>{ QBrush{ QColor{ 255, 100, 100 } } };
  QTest::newRow("overridden")
    << QCMakePropertyList{ makeProperty("VARIABLE_1", "Value 1") }
    << QCMakePropertyList{ makeProperty("VARIABLE_1", "Overridden value") }
    << QVector<QString>{ "VARIABLE_1" }
    << QVector<QString>{ "Overridden value" }
    << QVector<QVariant>{ QVariant{} };
  QTest::newRow("overriddenMixed")
    << QCMakePropertyList{ makeProperty("VARIABLE_1", "Value 1") }
    << QCMakePropertyList{ makeProperty("VARIABLE_1", "Overridden value"),
                           makeProperty("VARIABLE_2", "Value 2") }
    << QVector<QString>{ "VARIABLE_2", "VARIABLE_1" }
    << QVector<QString>{ "Value 2", "Overridden value" }
    << QVector<QVariant>{ QBrush{ QColor{ 255, 100, 100 } }, QVariant{} };
}

QTEST_MAIN(QCMakeCacheModelTest)
