/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "EnvironmentDialogTest.h"

#include <QDialogButtonBox>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QString>
#include <QtTest>

#include "CatchShow.h"
#include "EnvironmentDialog.h"

EnvironmentDialogTest::EnvironmentDialogTest(QObject* parent)
  : QObject(parent)
{
}

void EnvironmentDialogTest::environmentDialog()
{
  CatchShow catcher;
  catcher.setCallback<QMessageBox>([](QMessageBox* box) { box->accept(); });

  QProcessEnvironment env;
  env.insert("DELETED_VARIABLE_1", "Deleted variable 1");
  env.insert("DELETED_VARIABLE_2", "Deleted variable 2");
  env.insert("KEPT_VARIABLE", "Kept variable");
  env.insert("CHANGED_VARIABLE", "This will be changed");

  EnvironmentDialog dialog(env);

  {
    QStringList expected{
      "CHANGED_VARIABLE=This will be changed",
      "DELETED_VARIABLE_1=Deleted variable 1",
      "DELETED_VARIABLE_2=Deleted variable 2",
      "KEPT_VARIABLE=Kept variable",
    };
    QCOMPARE(dialog.environment().toStringList(), expected);
    QCOMPARE(catcher.count(), 0);
  }

  {
    CatchShow catcher2;
    bool done = false;
    catcher2.setCallback<QDialog>([&catcher, &done](QDialog* box) {
      if (done) {
        return;
      }
      done = true;

      auto name = box->findChild<QLineEdit*>("name");
      auto value = box->findChild<QLineEdit*>("value");
      auto acceptReject = box->findChild<QDialogButtonBox*>();

      name->setText("");
      value->setText("");
      acceptReject->button(QDialogButtonBox::Ok)->click();
      QCOMPARE(catcher.count(), 1);

      name->setText("KEPT_VARIABLE");
      value->setText("");
      acceptReject->button(QDialogButtonBox::Ok)->click();
      QCOMPARE(catcher.count(), 2);

      name->setText("ADDED_VARIABLE");
      value->setText("Added variable");
      acceptReject->button(QDialogButtonBox::Ok)->click();
      QCOMPARE(catcher.count(), 2);
    });
    dialog.AddEntry->click();

    QStringList expected{
      "ADDED_VARIABLE=Added variable",
      "CHANGED_VARIABLE=This will be changed",
      "DELETED_VARIABLE_1=Deleted variable 1",
      "DELETED_VARIABLE_2=Deleted variable 2",
      "KEPT_VARIABLE=Kept variable",
    };
    QCOMPARE(dialog.environment().toStringList(), expected);
    QCOMPARE(catcher.count(), 2);
    QVERIFY(done);
  }

  {
    CatchShow catcher2;
    bool done = false;
    catcher2.setCallback<QDialog>([&done](QDialog* box) {
      if (done) {
        return;
      }
      done = true;

      auto name = box->findChild<QLineEdit*>("name");
      auto value = box->findChild<QLineEdit*>("value");
      auto acceptReject = box->findChild<QDialogButtonBox*>();

      name->setText("DISCARDED_VARIABLE");
      value->setText("Discarded variable");
      acceptReject->button(QDialogButtonBox::Cancel)->click();
    });
    dialog.AddEntry->click();

    QStringList expected{
      "ADDED_VARIABLE=Added variable",
      "CHANGED_VARIABLE=This will be changed",
      "DELETED_VARIABLE_1=Deleted variable 1",
      "DELETED_VARIABLE_2=Deleted variable 2",
      "KEPT_VARIABLE=Kept variable",
    };
    QCOMPARE(dialog.environment().toStringList(), expected);
    QCOMPARE(catcher.count(), 2);
    QVERIFY(done);
  }

  {
    auto* model = dialog.Environment->model();
    auto* selectionModel = dialog.Environment->selectionModel();
    for (int i = 0; i < model->rowCount(); ++i) {
      auto index1 = model->index(i, 0);
      auto index2 = model->buddy(index1);
      auto name = model->data(index1, Qt::DisplayRole).toString();
      if (name == "DELETED_VARIABLE_1" || name == "DELETED_VARIABLE_2") {
        selectionModel->select(index1, QItemSelectionModel::Select);
        selectionModel->select(index2, QItemSelectionModel::Select);
      } else if (name == "CHANGED_VARIABLE") {
        model->setData(index2, "Changed variable", Qt::DisplayRole);
      }
    }
    dialog.RemoveEntry->click();

    QStringList expected{
      "ADDED_VARIABLE=Added variable",
      "CHANGED_VARIABLE=Changed variable",
      "KEPT_VARIABLE=Kept variable",
    };
    QCOMPARE(dialog.environment().toStringList(), expected);
  }
}

QTEST_MAIN(EnvironmentDialogTest)
