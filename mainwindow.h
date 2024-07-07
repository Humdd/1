#pragma once

#include "CDeviceProcess.h"
#include "FilmStripWindow.h"
#include "ICWindow.h"
#include "header.h"
#include "tcp_server.h"
#include <QDebug>
#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <algorithm>

#define DEVICE_COUNTS 2 //最多允许同时操作2台设备

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //测试显示图片
    void testShowImage();

private:
    Ui::MainWindow *ui;

    //初始化界面UI
    void __InitUI(CGXFeatureControlPointer objFeatureControlPtr);

    //更新界面
    void __UpdateUI(CDeviceProcess *pDeviceProcess);

    //关闭窗口
    void closeEvent(QCloseEvent *event);

    //初始化API
    void __InitCGXAPI();

    //检查相机是否打开
    bool checkDevice();

    //处理客户端发来的信息
    void processRecvMsg(QString recvMsg);

    //打开IC标注工具
    void openICAnnotation();

    //打开热合标注工具
    void openHeatAnnotation();

    GxIAPICPP::gxdeviceinfo_vector m_vectorDeviceInfo;      // 枚举到的设备信息
    CDeviceProcess *m_pDeviceProcess[DEVICE_COUNTS];        // 设备处理类对象数组
    CDeviceProcess *m_pDeviceProcessCurrent;                // 当前设备处理类对象
    std::map<int, CDeviceProcess *> m_mapDeviceInformation; // 用于存储设备处理类
    int m_nDeviceListCurrent;                               // 当前设备列表序号
    CGXBitmap *pLBitmap; //显示区域左窗口
    CGXBitmap *pRBitmap;
    QTextCharFormat format;
    QTextCursor textCursor;          //日志窗口句柄
    ICWindow iCWindow;               //IC窗口对象
    FilmStripWindow filmStripWindow; //热合窗口对象
    TcpServer *tcp_server_;          //服务器
    QTcpSocket *current_socket;
    QTcpSocket *socket_ic;
    QTcpSocket *socket_heat;
    int cnt_client;                       //socket连接数量
    bool m_bIsConnected;                  //是否建立Tcp连接标志
    bool m_bIsCmdFromLocal;               //命令是否本地触发的
    bool m_bIsInTestMode;                 //是否处于检测模式
    bool m_bCheckShowDevice;              //是否显示设备序列号
    QLabel *m_pLLabelStaBar;              //状态栏
    QLabel *m_pRLabelStaBar;

signals:
    void ServerRecved(qintptr, QTcpSocket *, const QByteArray &);
private slots:
    void ClientConnected(qintptr handle, QTcpSocket *socket);
    void ClientDisconnected(qintptr handle);
    /**
     * @brief 服务端收到消息的信号
     *        若想要统一管理或做日志处理可连接此信号
     * @param 收到消息的连接句柄
     * @param 收到消息的socket指针
     * @param 收到的消息内容
     */
    void ServerRecvedSlot(qintptr handle, QTcpSocket *socket, const QByteArray &data);
private slots:
    void on_Cb_DeviceList_currentIndexChanged(int nIndex);  //切换设备列表
    void on_pB_StartServer_clicked(); //启动服务器，集成打开设备和开始采集
    void on_pB_StopServer_clicked();  //关闭服务器，集成关闭采集和关闭设备
    void on_ckB_SaveImage_toggled(bool checked);
    void on_ckB_ShowImage_toggled(bool checked);
    void on_ckB_TestMode_toggled(bool checked); //切换为检测模式
    void on_ckB_ShowDeviceSN_toggled(bool checked); //显示设备序列号
    void on_pB_refreshDeviceList_clicked();  //更新设备列表
    void sendMsg(QString sendMsg); //发送消息到socket
    void writeLog(const QString &str, const int mode); //写日志到日志窗口，mode：0正常，1异常
    void on_pushButton_SendCommand_clicked();
};
