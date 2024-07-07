#pragma once
#include "header.h"
#include "cgxbitmap.h"
#include <QDateTime>
#include "utils.h"
#include <QTcpSocket>


class CSampleCaptureEventHandler;

//---------------------------------------------------------------------------------
/**
\brief   设备处理类
*/
//----------------------------------------------------------------------------------
class CDeviceProcess : public QObject{
    Q_OBJECT
public:

    CDeviceProcess();
    ~CDeviceProcess();

signals:
    //发送socket消息
    void send_to_mainwind(QString sendMsg);
    //写日志到日志窗口
    void writeLog(QString, int);
public slots:
    void processImage(QImage, QString);

public:
    // 打开设备
    void OpenDevice(gxstring strDeviceSN, CGXBitmap *m_pBitmap, int nDeviceIndex);

    // 关闭设备
    void CloseDevice();

    // 开始采集
    void StartSnap();

    // 停止采集
    void StopSnap();

    void setBitmap(CGXBitmap *pBitmap);

    // 获取设备打开标识
    bool IsOpen();

    // 获取设备采集标识
    bool IsSnap();

    // 获取设备打开和采集标识
    bool IsOpenAndSnap();

    // 设置是否显示帧率标志
    void SetShowSN(bool bIsShowFps);

    // 设置是否显示图像标志
    void setShowImage(bool bShowImage);

    // 设置显示图像帧间隔
    void setShowFrameInterval(int nShowFrameInterval);

    // 设置保存图像帧间隔
    void setSaveFrameInterval(int nSaveFrameInterval);

    // 设置保存图像列表大小
    void setSaveFileNameListSize(int nSaveFileNameListSize);

    // 获取是否显示帧率标志
    bool GetShowSN();

    // 刷新设备型号及帧率信息
    void RefreshDeviceSN();

    // 设置是否保存图像标志
    void setSaveImage(bool bSaveImage);

    // 设置是否从socket命令保存图像标志
    void setSaveICImageFromSocket(bool bSaveImageFromSocket);
    bool getSaveICImageFromSocket()
    {
        return m_bSaveICImageFromSocket;
    }

    void setSaveHeatImageFromSocket(bool bSaveImageFromSocket);
    bool getSaveHeatImageFromSocket()
    {
        return m_bSaveHeatImageFromSocket;
    }

    void setDetectICFromSocket(bool bDetectICFFromSocket);
    bool getDetectICFromSocket()
    {
        return m_bDetectICFromSocket;
    }

    void setDetectHeatFromSocket(bool bDetectHeatFromSocket);
    bool getDetectHeatFromSocket()
    {
        return m_bDetectHeatFromSocket;
    }

    // 保存图像
    void SavePicture(CImageDataPointer &objImageDataPointer); //deprecated

    void saveImage(QImage &image, QString &strFileName);

    //检测模式下，保存图像用于标注
    void saveImageForAnnotationInTestMode(const QString &);
    void saveImageForAnnotationInTestModeCallback(QImage &, const QString &, const QString &);

    //设置触发模式，mode为 On或 Off
    void setTriggerMode(GxIAPICPP::gxstring mode)
    {
        m_objFeatureControlPtr->GetEnumFeature("TriggerMode")->SetValue(mode);
    }
    void triggerSoftware();

    //非检测模式下，循环显示图片
    void showRecursivelyNotInTestMode();

    void setTestMode(bool inTestMode)
    {
        m_bInTestMode = inTestMode;
    }
    bool getTestMode()
    {
        return m_bInTestMode;
    }

    //检测
    void detectInTestMode(const QString &);
    void detectInTestModeCallback(QImage &, const QString &, const QString &);

    //deprecated
    void savePicture(cv::Mat &image, QString strFileName);

    void showImage(const QImage &image, QString strDeviceSN);

    // 设置设备序号
    void setDeviceIndex(int index);
    int getDeviceIndex()
    {
        return m_nDeviceIndex;
    }

    QString getDeviceSN()
    {
        return QString(m_strDeviceSN);
    }

    // 是否需要创建图像
    bool needCreateImage();

    // 获取朝向类型
    int getType(cv::Mat &src);

    CGXFeatureControlPointer m_objFeatureControlPtr;       ///< 属性控制器
    CGXFeatureControlPointer m_objStreamFeatureControlPtr; ///< 流层控制器对象

private:
    /// 显示图像
    void __ShowPicture(CImageDataPointer &objImageDataPointer);

    CGXDevicePointer            m_objDevicePtr;                  //设备句柄
    CGXStreamPointer            m_objStreamPtr;                  //设备流对象
    CGXBitmap                  *m_pBitmap;                       //画图指针
    CSampleCaptureEventHandler *m_pCSampleCaptureEventHandler;   //回调指针
    bool                        m_bIsOpen;                       //设备是否打开标志
    bool                        m_bIsSnap;                       //设备是否开采标志
    bool                        m_bIsOpenAndSnap;                //设备是否打开和采集标志
    bool                        m_bShowDeviceSN;                 //显示设备型号标志
    bool                        m_bShowImage;                    //是否显示图像标志
    bool                        m_bInTestMode;                     //是否在检测模式
    bool                        m_bSaveImage;                    //是否保存图像标志
    bool                        m_bSaveICImageFromSocket;        //根据socket命令是否保存ic图像标志
    bool                        m_bSaveHeatImageFromSocket;      //根据socket命令是否保存heat图像标志
    bool                        m_bDetectICFromSocket;           //根据socket命令检测IC
    bool                        m_bDetectHeatFromSocket;         //根据socket命令检测热合
    char                        *m_pRecordDeviceSN;              //设备型号信息
    gxstring                    m_strDeviceSN;                   //设备序列号
    QString                     m_strSavePath;                   //图像保存路径
    string                      m_strFileName;                   //保存图像的名称
    queue<QString>              m_qSaveFileNameList;             //保存图像名称列表
    int                         m_nDeviceIndex;                  //设备序号
    int                         m_nSaveFileNameListSize;         //保存图像名称列表大小
    int                         m_nSaveFrameInterval;            //保存图片帧间隔
    int                         m_nShowFrameInterval;            //显示图片帧间隔
    int                         nTmpSaveFrameInterval;
    int                         nTmpShowFrameInterval;

    Utils utils; //工具类，判断芯片朝向类型，检测logo
};

//---------------------------------------------------------------------------------
/**
\brief   自定义采集事件处理类
*/
//----------------------------------------------------------------------------------
class CSampleCaptureEventHandler : public QObject, public ICaptureEventHandler
{
    Q_OBJECT
    //---------------------------------------------------------------------------------
    /**
    \brief   采集回调函数
    \param   objImageDataPointer      图像处理参数
    \param   pFrame                   用户参数
    \return  无
    */
    //----------------------------------------------------------------------------------
    void DoOnImageCaptured(CImageDataPointer &objImageDataPointer, void *pUserParam);

signals:
    void imageReady(QImage, QString);
}; //class CSampleCaptureEventHandler


