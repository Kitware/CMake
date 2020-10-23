/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "QCMakePresetTest.h"

#include <utility>

#include "QCMakePreset.h"
#include <QtTest>

namespace {
QCMakePreset makePreset()
{
  return QCMakePreset{
    /*name=*/"name",
    /*displayName=*/"displayName",
    /*description=*/"description",
    /*generator=*/"generator",
    /*architecture=*/"architecture",
    /*setArchitecture=*/true,
    /*toolset=*/"toolset",
    /*setToolset=*/true,
    /*enabled=*/true,
  };
}

template <typename T, typename U>
QCMakePreset makePreset(T QCMakePreset::*field, U&& value)
{
  auto preset = makePreset();
  preset.*field = std::forward<U>(value);
  return preset;
}
}

void QCMakePresetTest::equality()
{
  QFETCH(QCMakePreset, rhs);
  QFETCH(bool, equal);
  QFETCH(bool, lt);
  QFETCH(bool, gt);

  auto lhs = makePreset();
  QVERIFY((lhs == rhs) == equal);
  QVERIFY((lhs != rhs) == !equal);
  QVERIFY((lhs < rhs) == lt);
  QVERIFY((lhs >= rhs) == !lt);
  QVERIFY((lhs > rhs) == gt);
  QVERIFY((lhs <= rhs) == !gt);
}

void QCMakePresetTest::equality_data()
{
  QTest::addColumn<QCMakePreset>("rhs");
  QTest::addColumn<bool>("equal");
  QTest::addColumn<bool>("lt");
  QTest::addColumn<bool>("gt");

  QTest::newRow("equal") << makePreset() << true << false << false;
  QTest::newRow("name") << makePreset(&QCMakePreset::name, "other-name")
                        << false << true << false;
  QTest::newRow("displayName")
    << makePreset(&QCMakePreset::displayName, "other-displayName") << false
    << true << false;
  QTest::newRow("description")
    << makePreset(&QCMakePreset::description, "other-description") << false
    << true << false;
  QTest::newRow("generator")
    << makePreset(&QCMakePreset::generator, "other-generator") << false << true
    << false;
  QTest::newRow("architecture")
    << makePreset(&QCMakePreset::architecture, "other-architecture") << false
    << true << false;
  QTest::newRow("setArchitecture")
    << makePreset(&QCMakePreset::setArchitecture, false) << false << false
    << true;
  QTest::newRow("toolset") << makePreset(&QCMakePreset::toolset,
                                         "other-toolset")
                           << false << false << true;
  QTest::newRow("setToolset")
    << makePreset(&QCMakePreset::setToolset, false) << false << false << true;
  QTest::newRow("enabled") << makePreset(&QCMakePreset::enabled, false)
                           << false << false << true;
}

QTEST_MAIN(QCMakePresetTest)
