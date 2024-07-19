#include "widget1.h"

#include "src/ui_widget1.h"

Widget1::Widget1(QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::Widget1)
{
  ui->setupUi(this);
  connect(ui->lineEdit, SIGNAL(textChanged(const QString&)), this,
          SLOT(onTextChanged(const QString&)));
}

Widget1::~Widget1()
{
  delete ui;
}

void Widget1::onTextChanged(const QString& text)
{
  ui->OnTextChanged->setText(text);
}
