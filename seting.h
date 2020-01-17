#ifndef SETING_H
#define SETING_H

#include <QWidget>
#include <QButtonGroup>
#include <QSettings>

namespace Ui {
class seting;
}

class seting : public QWidget
{
    Q_OBJECT

public:
    explicit seting(QWidget *parent = 0);
    ~seting();

    QButtonGroup* bgGroup;

private slots:
    void on_pushButton_cancel_clicked();

    void on_pushButton_ok_clicked();

private:
    Ui::seting *ui;
};

#endif // SETING_H
