/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "QCMakePresetItemModelTest.h"

#include <utility>

#include "QCMakePreset.h"
#include "QCMakePresetItemModel.h"
#include <QHash>
#include <QMetaType>
#include <QSignalSpy>
#include <QVariant>
#include <QVector>
#include <QtTest>

using QItemDataHash = QHash<Qt::ItemDataRole, QVariant>;

void QCMakePresetItemModelTest::initTestCase()
{
  QMetaType::registerComparators<QCMakePreset>();
}

void QCMakePresetItemModelTest::initTestCase_data()
{
  QTest::addColumn<QVector<QCMakePreset>>("presets");
  QTest::addColumn<QVector<QItemDataHash>>("data");

  QVector<QCMakePreset> presets{
    QCMakePreset{
      /*name=*/"no-description",
      /*description=*/"",
      /*description=*/"",
      /*generator=*/"",
      /*architecture=*/"",
      /*setArchitecture=*/true,
      /*toolset=*/"",
      /*setToolset=*/true,
      /*enabled=*/true,
    },
    QCMakePreset{
      /*name=*/"short-description",
      /*description=*/"Short Description",
      /*description=*/"",
      /*generator=*/"",
      /*architecture=*/"",
      /*setArchitecture=*/true,
      /*toolset=*/"",
      /*setToolset=*/true,
      /*enabled=*/true,
    },
    QCMakePreset{
      /*name=*/"long-description",
      /*description=*/"",
      /*description=*/"Long Description",
      /*generator=*/"",
      /*architecture=*/"",
      /*setArchitecture=*/true,
      /*toolset=*/"",
      /*setToolset=*/true,
      /*enabled=*/true,
    },
    QCMakePreset{
      /*name=*/"disabled",
      /*description=*/"",
      /*description=*/"",
      /*generator=*/"",
      /*architecture=*/"",
      /*setArchitecture=*/true,
      /*toolset=*/"",
      /*setToolset=*/true,
      /*enabled=*/false,
    },
  };
  QVector<QItemDataHash> data{
    QItemDataHash{
      { Qt::AccessibleDescriptionRole, "" },
      { Qt::DisplayRole, "no-description" },
      { Qt::ToolTipRole, "" },
      { Qt::UserRole, QVariant::fromValue(presets[0]) },
      { Qt::FontRole, QFont{} },
    },
    QItemDataHash{
      { Qt::AccessibleDescriptionRole, "" },
      { Qt::DisplayRole, "Short Description" },
      { Qt::ToolTipRole, "" },
      { Qt::UserRole, QVariant::fromValue(presets[1]) },
      { Qt::FontRole, QFont{} },
    },
    QItemDataHash{
      { Qt::AccessibleDescriptionRole, "" },
      { Qt::DisplayRole, "long-description" },
      { Qt::ToolTipRole, "Long Description" },
      { Qt::UserRole, QVariant::fromValue(presets[2]) },
      { Qt::FontRole, QFont{} },
    },
    QItemDataHash{
      { Qt::AccessibleDescriptionRole, "" },
      { Qt::DisplayRole, "disabled" },
      { Qt::ToolTipRole, "" },
      { Qt::UserRole, QVariant::fromValue(presets[3]) },
      { Qt::FontRole, QFont{} },
    },
    QItemDataHash{
      { Qt::AccessibleDescriptionRole, "separator" },
      { Qt::DisplayRole, QVariant{} },
      { Qt::ToolTipRole, QVariant{} },
      { Qt::UserRole, QVariant{} },
      { Qt::FontRole, QFont{} },
    },
    QItemDataHash{
      { Qt::AccessibleDescriptionRole, "" },
      { Qt::DisplayRole, "<custom>" },
      { Qt::ToolTipRole, "Specify all settings manually" },
      { Qt::UserRole, QVariant{} },
      { Qt::FontRole,
        []() {
          QFont f;
          f.setItalic(true);
          return f;
        }() },
    },
  };
  QTest::newRow("many") << presets << data;
  QTest::newRow("none") << QVector<QCMakePreset>{}
                        << QVector<QItemDataHash>{ data.last() };
}

void QCMakePresetItemModelTest::data()
{
  QFETCH_GLOBAL(QVector<QCMakePreset>, presets);
  QFETCH_GLOBAL(QVector<QItemDataHash>, data);
  QFETCH(Qt::ItemDataRole, role);

  QCMakePresetItemModel model;
  QSignalSpy spy1(&model, &QCMakePresetItemModel::modelAboutToBeReset);
  QSignalSpy spy2(&model, &QCMakePresetItemModel::modelReset);
  model.setPresets(presets);
  QCOMPARE(spy1.size(), 1);
  QCOMPARE(spy2.size(), 1);

  QVector<QVariant> expectedData(data.size());
  for (int i = 0; i < data.size(); ++i) {
    expectedData[i] = data[i][role];
  }

  auto rows = model.rowCount();
  QVector<QVariant> actualData(rows);
  for (int i = 0; i < rows; ++i) {
    actualData[i] = model.data(model.index(i, 0), role);
  }

  QCOMPARE(actualData, expectedData);
}

void QCMakePresetItemModelTest::data_data()
{
  QTest::addColumn<Qt::ItemDataRole>("role");

  QTest::newRow("accessible") << Qt::AccessibleDescriptionRole;
  QTest::newRow("display") << Qt::DisplayRole;
  QTest::newRow("tooltip") << Qt::ToolTipRole;
  QTest::newRow("user") << Qt::UserRole;
  QTest::newRow("font") << Qt::FontRole;
}

QTEST_MAIN(QCMakePresetItemModelTest)
