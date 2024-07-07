#ifndef COMBOBOXTEST_H
#define COMBOBOXTEST_H

#include <QMainWindow>

namespace Ui {
class ComboBoxTest;
}

class ComboBoxTest : public QMainWindow
{
    Q_OBJECT

public:
    explicit ComboBoxTest(QWidget *parent = nullptr);
    ~ComboBoxTest();

private slots:
    void on_comboBox_currentIndexChanged(int index);

private:
    Ui::ComboBoxTest *ui;
};

#endif // COMBOBOXTEST_H
