#include "mainwindow.h"
#include <QApplication>
#include <QSettings>
#include <QString>


int main(int argc, char *argv[])
{
    QApplication* a = new QApplication(argc, argv);
    a->setQuitOnLastWindowClosed(false);//防止在关闭设置窗口时程序退出

    QSettings *configIniWrite = new QSettings("./ini/SerialInfo.ini", QSettings::IniFormat);
    int autoRun = configIniWrite->value("/set/autoRun",2).toInt();
    if(autoRun == 2)
    {
         configIniWrite->setValue("/set/autoRun","0");

    }
    delete configIniWrite;


    MainWindow w(a);
    w.show();

    return a->exec();
}
