/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "EnvironmentDialog.h"

#include "QCMakeWidgets.h"
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QStandardItem>

EnvironmentItemModel::EnvironmentItemModel(
  const QProcessEnvironment& environment, QObject* parent)
  : QStandardItemModel(parent)
{
  this->clear();
  for (auto const& key : environment.keys()) {
    auto value = environment.value(key);
    this->appendVariable(key, value);
  }
}

QProcessEnvironment EnvironmentItemModel::environment() const
{
  QProcessEnvironment env;
  for (int i = 0; i < this->rowCount(); ++i) {
    auto name = this->data(this->index(i, 0), Qt::DisplayRole).toString();
    auto value = this->data(this->index(i, 1), Qt::DisplayRole).toString();
    env.insert(name, value);
  }
  return env;
}

void EnvironmentItemModel::clear()
{
  this->QStandardItemModel::clear();

  QStringList labels;
  labels << tr("Name") << tr("Value");
  this->setHorizontalHeaderLabels(labels);
}

QModelIndex EnvironmentItemModel::buddy(const QModelIndex& index) const
{
  if (index.column() == 0) {
    return this->index(index.row(), index.column() + 1, index.parent());
  }
  return index;
}

void EnvironmentItemModel::appendVariable(const QString& key,
                                          const QString& value)
{
  this->insertVariable(this->rowCount(), key, value);
}

void EnvironmentItemModel::insertVariable(int row, const QString& key,
                                          const QString& value)
{
  for (int i = 0; i < this->rowCount(); ++i) {
    if (this->data(this->index(i, 0), Qt::DisplayRole) == key) {
      this->setData(this->index(i, 1), value, Qt::DisplayRole);
      return;
    }
  }

  auto* keyItem = new QStandardItem(key);
  auto* valueItem = new QStandardItem(value);
  this->insertRow(row, { keyItem, valueItem });
}

EnvironmentSearchFilter::EnvironmentSearchFilter(QObject* parent)
  : QSortFilterProxyModel(parent)
{
}

bool EnvironmentSearchFilter::filterAcceptsRow(int row,
                                               const QModelIndex& parent) const
{
  auto* model = this->sourceModel();
  auto key =
    model->data(model->index(row, 0, parent), Qt::DisplayRole).toString();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
  return key.contains(this->filterRegularExpression());
#else
  return key.contains(this->filterRegExp());
#endif
}

EnvironmentDialog::EnvironmentDialog(const QProcessEnvironment& environment,
                                     QWidget* parent)
  : QDialog(parent)
{
  this->setupUi(this);

  this->RemoveEntry->setEnabled(false);

  this->m_model = new EnvironmentItemModel(environment, this);
  this->m_filter = new EnvironmentSearchFilter(this);
  this->m_filter->setSourceModel(this->m_model);
  this->Environment->setModel(this->m_filter);

  this->Environment->setUniformRowHeights(true);
  this->Environment->setRootIsDecorated(false);
  this->Environment->setSelectionMode(QAbstractItemView::ExtendedSelection);
  this->Environment->setSelectionBehavior(QAbstractItemView::SelectRows);

  QObject::connect(this->AddEntry, &QAbstractButton::clicked, this,
                   &EnvironmentDialog::addEntry);
  QObject::connect(this->RemoveEntry, &QAbstractButton::clicked, this,
                   &EnvironmentDialog::removeSelectedEntries);
  QObject::connect(
    this->Search, &QLineEdit::textChanged, [this](const QString& text) {
      const bool valid = QtCMake::setSearchFilter(this->m_filter, text);
      QtCMake::setSearchFilterColor(this->Search, valid);
    });
  QObject::connect(this->Environment->selectionModel(),
                   &QItemSelectionModel::selectionChanged, this,
                   &EnvironmentDialog::selectionChanged);
}

QProcessEnvironment EnvironmentDialog::environment() const
{
  return this->m_model->environment();
}

void EnvironmentDialog::addEntry()
{
  // Build the dialog manually because it's simple enough
  QDialog dialog(this);
  dialog.setWindowTitle("Add Environment Variable");

  auto* layout = new QGridLayout;
  dialog.setLayout(layout);

  auto* nameLabel = new QLabel;
  nameLabel->setText("Name:");
  layout->addWidget(nameLabel, 0, 0);

  auto* nameEdit = new QLineEdit;
  nameEdit->setObjectName("name");
  layout->addWidget(nameEdit, 0, 1);

  auto* valueLabel = new QLabel;
  valueLabel->setText("Value:");
  layout->addWidget(valueLabel, 1, 0);

  auto* valueEdit = new QLineEdit;
  valueEdit->setObjectName("value");
  layout->addWidget(valueEdit, 1, 1);

  auto* buttons = new QDialogButtonBox;
  buttons->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  QObject::connect(
    buttons, &QDialogButtonBox::accepted, &dialog,
    [this, &dialog, nameEdit]() {
      auto text = nameEdit->text();
      if (text.isEmpty()) {
        QMessageBox::critical(&dialog, "Error", "Name must be non-empty.");
        return;
      }

      auto* model = this->Environment->model();
      for (int i = 0; i < model->rowCount(); ++i) {
        if (model->data(model->index(i, 0), Qt::DisplayRole) == text) {
          QMessageBox::critical(
            &dialog, "Error",
            tr("Environment variable \"%1\" already exists.").arg(text));
          return;
        }
      }

      dialog.accept();
    });
  QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog,
                   &QDialog::reject);
  layout->addWidget(buttons, 2, 0, 1, 2);

  if (dialog.exec() == QDialog::Accepted) {
    this->m_model->insertVariable(0, nameEdit->text(), valueEdit->text());
  }
}

void EnvironmentDialog::removeSelectedEntries()
{
  QModelIndexList idxs = this->Environment->selectionModel()->selectedRows();
  QList<QPersistentModelIndex> pidxs;
  foreach (QModelIndex const& i, idxs) {
    pidxs.append(i);
  }
  foreach (QPersistentModelIndex const& pi, pidxs) {
    this->Environment->model()->removeRow(pi.row(), pi.parent());
  }
}

void EnvironmentDialog::selectionChanged()
{
  auto selected = this->Environment->selectionModel()->selectedRows();
  this->RemoveEntry->setEnabled(!selected.isEmpty());
}
