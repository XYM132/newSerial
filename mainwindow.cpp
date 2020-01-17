#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qDebug>

#include <QAbstractNativeEventFilter>

#include <windows.h>
#include <dbt.h>
#include <devguid.h>
#include <SetupAPI.h>
#include <InitGuid.h>
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    on_pushButton_clicked();

    isClose=false;

    _icon = new QSystemTrayIcon();
    _menu = new QMenu();
    _icon->setIcon(QIcon(":/img/img/com.ico"));
    _icon->setToolTip("查看系统串口.");

    _menu->addAction("退出",this,SLOT(slotcloseApp()));

    _icon->setContextMenu(_menu);
    _icon->show();

    connect(_icon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(slotActived(QSystemTrayIcon::ActivationReason)));


#if 1//注册USB监听
    static const GUID GUID_DEVINTERFACE_LIST[] =
    {
        // GUID_DEVINTERFACE_USB_DEVICE
        { 0xA5DCBF10, 0x6530, 0x11D2,{ 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } },
        // GUID_DEVINTERFACE_DISK
        //{ 0x53f56307, 0xb6bf, 0x11d0,{ 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } },
        // GUID_DEVINTERFACE_HID,
        { 0x4D1E55B2, 0xF16F, 0x11CF,{ 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } },
        // GUID_NDIS_LAN_CLASS
        //{ 0xad498944, 0x762f, 0x11d0,{ 0x8d, 0xcb, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c } }
        //// GUID_DEVINTERFACE_COMPORT
        //{ 0x86e0d1e0, 0x8089, 0x11d0, { 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73 } },
        //// GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR
        //{ 0x4D36E978, 0xE325, 0x11CE, { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } },
        //// GUID_DEVINTERFACE_PARALLEL
        //{ 0x97F76EF0, 0xF883, 0x11D0, { 0xAF, 0x1F, 0x00, 0x00, 0xF8, 0x00, 0x84, 0x5C } },
        //// GUID_DEVINTERFACE_PARCLASS
        //{ 0x811FC6A5, 0xF728, 0x11D0, { 0xA5, 0x37, 0x00, 0x00, 0xF8, 0x75, 0x3E, 0xD1 } }
    };

    //注册插拔事件
    HDEVNOTIFY hDevNotify;
    DEV_BROADCAST_DEVICEINTERFACE NotifacationFiler;
    ZeroMemory(&NotifacationFiler, sizeof(DEV_BROADCAST_DEVICEINTERFACE));
    NotifacationFiler.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotifacationFiler.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

    for (int i = 0; i < sizeof(GUID_DEVINTERFACE_LIST) / sizeof(GUID); i++)
    {
        NotifacationFiler.dbcc_classguid = GUID_DEVINTERFACE_LIST[i];//GetCurrentUSBGUID();//m_usb->GetDriverGUID();

        hDevNotify = RegisterDeviceNotification((HANDLE)this->winId(), &NotifacationFiler, DEVICE_NOTIFY_WINDOW_HANDLE);
        if (!hDevNotify)
        {
            //int Err = GetLastError();
            qDebug() << "注册失败" << endl;
        }
        //else
    }
#endif

#if 1 //写入开机自启
    QString appName = QApplication::applicationName();//程序名称

    QString appPath = QApplication::applicationFilePath();// 程序路径

    appPath = appPath.replace("/","\\");

    QSettings *reg=new QSettings(
                "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                QSettings::NativeFormat);

    QString val = reg->value(appName).toString();// 如果此键不存在，则返回的是空字符串
    if(val != appPath)
        reg->setValue(appName,appPath);// 如果移除的话，reg->remove(applicationName);

    reg->deleteLater();

    this->hide();
#endif

    this->setWindowFlags(Qt::FramelessWindowHint);

    QRect screenRect = QApplication::desktop()->availableGeometry();
    this->setGeometry(QRect(screenRect.width()-this->width(),
                            screenRect.height()-this->height(),
                            this->width(),
                            this->height()));


    m_timer = new QTimer();
    m_timer->setInterval(10000);
    m_timer->setSingleShot(true);
    connect(m_timer,SIGNAL(timeout()),this,SLOT(TimerTimeOut()));
    m_timer->start();
    this->installEventFilter(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}




void MainWindow::on_pushButton_clicked()
{
    QSerialPort* serial = new QSerialPort();
    ui->textEdit_new->clear();
    ui->textEdit_all->clear();
    foreach(const QSerialPortInfo &portinfo,QSerialPortInfo::availablePorts())
       {
           serial->setPort(portinfo); //此处的serial是在构造函数中QSerialPort 的对象；
           if(qFind(allCom.begin(),allCom.end(),serial->portName()) == allCom.end())
           {
               ui->textEdit_new->append(serial->portName() +" : "+portinfo.description());
               allCom.push_back(serial->portName());

           }
           ui->textEdit_all->append(serial->portName() +" : "+portinfo.description());
       }
}


bool MainWindow::nativeEvent(const QByteArray & eventType, void * message, long *result)
{

    Q_UNUSED(eventType);
    QString DevPathName;
    MSG* msg = reinterpret_cast<MSG*>(message);
    int msgType = msg->message;
    if (msgType == WM_DEVICECHANGE)
    {
        PDEV_BROADCAST_DEVICEINTERFACE lpdb = (PDEV_BROADCAST_DEVICEINTERFACE)msg->lParam;
        switch (msg->wParam)
        {
        case DBT_DEVICEARRIVAL:   // usb insert

            if (lpdb->dbcc_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
            {

                DevPathName = QString::fromWCharArray(lpdb->dbcc_name); // wchar* to QString

                qDebug()<<"DBT_DEVICEARRIVAL";

                //判断是否插入的设备是串口
                QSerialPort* serial = new QSerialPort();
                foreach(const QSerialPortInfo &portinfo,QSerialPortInfo::availablePorts())
                {
                   serial->setPort(portinfo); //此处的serial是在构造函数中QSerialPort 的对象；
                   if(qFind(allCom.begin(),allCom.end(),serial->portName()) == allCom.end())
                   {
                       on_pushButton_clicked();
                       this->show();
                       m_timer->start();
                       break;
                   }
                }

#if 0
                if ((MyDevPathName.contains(DevPathName, Qt::CaseInsensitive)) && (InterfaceSelect() == PORT_USB)) {
                    OnBnClickedBtnOpenDevice();
                    //AddToInfOut(_T("Device connected"),1,1);
                }
#endif
            }

            break;
        case DBT_DEVICEREMOVECOMPLETE:    // usb remove
            if (lpdb->dbcc_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
            {

                DevPathName = QString::fromWCharArray(lpdb->dbcc_name); // wchar* to QString
                qDebug()<<"DBT_DEVICEREMOVECOMPLETE";


                //更新串口号的容器
                allCom.clear();
                QSerialPort* serial = new QSerialPort();
                foreach(const QSerialPortInfo &portinfo,QSerialPortInfo::availablePorts())
                {
                   serial->setPort(portinfo); //此处的serial是在构造函数中QSerialPort 的对象；
                   allCom.push_back(serial->portName());
                }


#if 0
                if ((MyDevPathName.contains(DevPathName, Qt::CaseInsensitive)) && (InterfaceSelect() == PORT_USB)) {
                    OnBnClickedButtonClose();
                    //AddToInfOut(_T("Device connected"),1,1);
                }
#endif
            }
            break;
        default:
            break;
        }
    }
    return false;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(isClose){
        _icon->deleteLater();
        event->accept();
     }

    else {
        event->ignore();
        this->hide();
    }

}


void MainWindow::on_pushButton_2_clicked()
{
    this->hide();
}

void MainWindow::slotActived(QSystemTrayIcon::ActivationReason res)
{
    if(res==QSystemTrayIcon::Trigger)
    {
        this->show();
    }
}

void MainWindow::slotcloseApp()
{
    isClose=true;
    this->close();
}

void MainWindow::TimerTimeOut()
{
    this->hide();
}


void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_Drag = true;
        m_DragPosition = event->globalPos() - this->pos();
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_Drag && (event->buttons() && Qt::LeftButton)) {
        move(event->globalPos() - m_DragPosition);
        event->accept();
    }

}

void MainWindow::mouseReleaseEvent(QMouseEvent *)
{
    m_Drag = false;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::HoverMove)
        m_timer->start();

    return QMainWindow::eventFilter(watched,event);
}
