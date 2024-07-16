#include "mainwindow.h"

#include <QVBoxLayout>

#include "src/ui_mainwindow.h"
#include "widget1.h"

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  auto layout = new QVBoxLayout;
  layout->addWidget(new Widget1);

  QWidget* w = new QWidget(this);
  w->setLayout(layout);

  setCentralWidget(w);
}

MainWindow::~MainWindow()
{
  delete ui;
}
