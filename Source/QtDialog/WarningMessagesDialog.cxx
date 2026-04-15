/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "WarningMessagesDialog.h"

#include "cm/string_view"

#include <QButtonGroup>
#include <QHeaderView>
#include <QRadioButton>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "cmDiagnostics.h"

#include "QCMake.h"
#include "QCMakeSizeType.h"

WarningMessagesDialog::WarningMessagesDialog(QWidget* prnt, QCMake* instance)
  : QDialog(prnt)
  , cmakeInstance(instance)
{
  this->setupUi(this);
  this->setInitialValues();
  this->setupSignals();
}

void WarningMessagesDialog::setInitialValues()
{
  QHeaderView* const header = this->treeWidget->header();
  header->setSectionResizeMode(0, QHeaderView::Stretch);
  header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  header->setSectionResizeMode(3, QHeaderView::ResizeToContents);

  std::map<unsigned, QTreeWidgetItem*> items = {
    { 0, this->treeWidget->invisibleRootItem() },
  };

  for (unsigned i = 1; i < cmDiagnostics::CategoryCount; ++i) {
    auto const category = static_cast<cmDiagnosticCategory>(i);
    cm::string_view const cname =
      cmDiagnostics::GetCategoryString(category).substr(4);
    QTreeWidgetItem* const parent =
      items[cmDiagnostics::CategoryInfo[category].Parent];

    QTreeWidgetItem* const item = new QTreeWidgetItem(parent);
    item->setText(0,
                  QString::fromUtf8(cname.data(),
                                    static_cast<cm_qsizetype>(cname.size())));

    auto makeButton = [](QString const& text) {
      QToolButton* const button = new QToolButton;
      button->setCheckable(true);
      button->setText(text);
      return button;
    };
    QAbstractButton* const ignore = makeButton(tr("Ignore"));
    QAbstractButton* const warn = makeButton(tr("Warn"));
    QAbstractButton* const error = makeButton(tr("Error"));

    QButtonGroup* const buttonGroup = new QButtonGroup(treeWidget);
    buttonGroup->addButton(ignore, cmDiagnosticAction::Ignore);
    buttonGroup->addButton(warn, cmDiagnosticAction::Warn);
    buttonGroup->addButton(error, cmDiagnosticAction::SendError);
    buttonGroup->setExclusive(true);
    this->buttons.emplace(i, buttonGroup);

    treeWidget->setItemWidget(item, 1, ignore);
    treeWidget->setItemWidget(item, 2, warn);
    treeWidget->setItemWidget(item, 3, error);

    cmDiagnosticAction const action =
      this->cmakeInstance->getDiagnosticAction(category);
    switch (action) {
      case cmDiagnostics::SendError:
      case cmDiagnostics::FatalError:
        error->setChecked(true);
        break;
      case cmDiagnostics::Warn:
        warn->setChecked(true);
        break;
      default:
        ignore->setChecked(true);
        break;
    }

    items.emplace(i, item);
  }

  for (auto const& ii : items) {
    ii.second->setExpanded(true);
  }
}

void WarningMessagesDialog::setupSignals()
{
  QObject::connect(this->buttonBox, &QDialogButtonBox::accepted, this,
                   &WarningMessagesDialog::doAccept);
}

void WarningMessagesDialog::doAccept()
{
  for (auto const& ii : this->buttons) {
    this->cmakeInstance->setDiagnosticAction(
      static_cast<cmDiagnosticCategory>(ii.first),
      static_cast<cmDiagnosticAction>(ii.second->checkedId()));
  }
}
