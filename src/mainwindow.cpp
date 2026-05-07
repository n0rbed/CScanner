#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    lexical.Run();
    ui->setupUi(this);
    QString qStr = QString::fromStdString(lexical.getSymbolTable().print());
    QString qStrSourceCode = QString::fromStdString(lexical.getScanner().getSourceString());
    ui->sourceCode->setText(qStrSourceCode);
    ui->symbolTable->setText(qStr);
}

MainWindow::~MainWindow()
{
    delete ui;
}
