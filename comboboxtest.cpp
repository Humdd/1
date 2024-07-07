#include "comboboxtest.h"
#include "ui_comboboxtest.h"
#include <QDebug>

ComboBoxTest::ComboBoxTest(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ComboBoxTest)
{
    ui->setupUi(this);
    ui->comboBox->addItem("A01234");
    ui->comboBox->addItem("B56789");
}

ComboBoxTest::~ComboBoxTest()
{
    delete ui;
}

void ComboBoxTest::on_comboBox_currentIndexChanged(int index)
{
    qDebug() << index;
}

