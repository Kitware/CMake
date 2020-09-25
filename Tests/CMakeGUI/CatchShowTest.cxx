/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "CatchShowTest.h"

#include <QMessageBox>
#include <QtTest>

#include "CatchShow.h"

CatchShowTest::CatchShowTest(QObject* parent)
  : QObject(parent)
{
}

void CatchShowTest::catchShow()
{
  bool have = false;
  CatchShow catcher;
  catcher.setCallback<QMessageBox>([&have](QMessageBox* box) {
    have = true;
    box->accept();
  });

  QCOMPARE(catcher.count(), 0);
  QCOMPARE(have, false);

  {
    QDialog dialog;
    dialog.show();
    QCOMPARE(catcher.count(), 0);
    QCOMPARE(have, false);
  }

  {
    have = false;
    QMessageBox::critical(nullptr, "Error", "This is an error");
    QCOMPARE(catcher.count(), 1);
    QCOMPARE(have, true);
  }

  {
    have = false;
    QMessageBox::information(nullptr, "Info", "This is information");
    QCOMPARE(catcher.count(), 2);
    QCOMPARE(have, true);
  }
}

QTEST_MAIN(CatchShowTest)
