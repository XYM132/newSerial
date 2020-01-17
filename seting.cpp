#include "seting.h"
#include "ui_seting.h"
#include <qDebug>
seting::seting(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::seting)
{
    ui->setupUi(this);

    QSettings *configIniWrite = new QSettings("./ini/SerialInfo.ini", QSettings::IniFormat);
    int autoRun = configIniWrite->value("/set/autoRun",2).toInt();
    delete configIniWrite;

    if(autoRun == 1)
        ui->radioButton_open->click();
    else
        ui->radioButton_close->click();


    bgGroup = new QButtonGroup(this);
    bgGroup->addButton(ui->radioButton_open,1);
    bgGroup->addButton(ui->radioButton_close,2);
}

seting::~seting()
{
    delete ui;
}

void seting::on_pushButton_cancel_clicked()
{
    this->close();
}

void seting::on_pushButton_ok_clicked()
{
    QSettings *configIniWrite = new QSettings("./ini/SerialInfo.ini", QSettings::IniFormat);

    QString appName = QApplication::applicationName();//程序名称
    QString appPath = QApplication::applicationFilePath();// 程序路径
    appPath = appPath.replace("/","\\");
    QSettings *reg=new QSettings(
                "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                QSettings::NativeFormat);
    QString val = reg->value(appName).toString();// 如果此键不存在，则返回的是空字符串)



    if(bgGroup->checkedId() == 1)
    {
        configIniWrite->setValue("/set/autoRun","1");
//        if(val != appPath)
        reg->setValue(appName,appPath);// 如果移除的话，reg->remove(applicationName);
    }
    else
    {
        configIniWrite->setValue("/set/autoRun","0");
        reg->remove(appName);
    }

    delete configIniWrite;

    reg->deleteLater();



    this->close();
}

