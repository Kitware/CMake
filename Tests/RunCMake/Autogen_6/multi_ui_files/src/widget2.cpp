#include "widget2.h"

#include "src/ui_widget2.h"

Widget2::Widget2(QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::Widget2)
{
  ui->setupUi(this);
  connect(ui->lineEdit, SIGNAL(textChanged(const QString&)), this,
          SLOT(onTextChanged(const QString&)));
}

Widget2::~Widget2()
{
  delete ui;
}

void Widget2::onTextChanged(const QString& text)
{
  ui->OnTextChanged->setText(text);
}
