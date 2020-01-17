#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QCloseEvent>
#include <QVector>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QSettings>
#include <QTimer>

namespace Ui {
class MainWindow;
}



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();
    void slotActived(QSystemTrayIcon::ActivationReason res);
    void slotcloseApp();

    void TimerTimeOut();

private:
    Ui::MainWindow *ui;
    QVector<QString> allCom;
    bool isClose;
    bool m_Drag;
    QPoint m_DragPosition;
    QSystemTrayIcon* _icon;
    QMenu* _menu;

    QTimer* m_timer;



protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
    void closeEvent(QCloseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
    bool eventFilter(QObject *watched, QEvent *event);
};

#endif // MAINWINDOW_H
