#include "CDeviceProcess.h"
#include <windows.h>
#include "ICWindow.h"
#include "FilmStripWindow.h"

CDeviceProcess::CDeviceProcess()
    : m_pBitmap(nullptr)
    , m_pCSampleCaptureEventHandler(nullptr)
    , m_bIsOpen(false)
    , m_bIsSnap(false)
    , m_bIsOpenAndSnap(false)
    , m_bShowDeviceSN(true)
    , m_bShowImage(true)
    , m_bInTestMode(false)
    , m_bSaveICImageFromSocket(false)
    , m_bDetectICFromSocket(false)
    , m_bDetectHeatFromSocket(false)
    , m_pRecordDeviceSN(new char[1024])
    , m_strDeviceSN("")
    , m_strSavePath("./images/")
    , m_nDeviceIndex(0)
    , m_nSaveFileNameListSize(0)
    , m_nSaveFrameInterval(1)
    , m_nShowFrameInterval(1)
    , nTmpSaveFrameInterval(1)
    , nTmpShowFrameInterval(1)
{
    m_pCSampleCaptureEventHandler = new CSampleCaptureEventHandler();
    connect(m_pCSampleCaptureEventHandler, &CSampleCaptureEventHandler::imageReady, this, &CDeviceProcess::processImage, Qt::QueuedConnection);
}

CDeviceProcess::~CDeviceProcess() {
    if (m_pCSampleCaptureEventHandler != nullptr)
    {
        delete m_pCSampleCaptureEventHandler;
        m_pCSampleCaptureEventHandler = nullptr;
    }

    if (m_pRecordDeviceSN != nullptr)
    {
        delete[] m_pRecordDeviceSN;
        m_pRecordDeviceSN = nullptr;
    }
}

void CDeviceProcess::processImage(QImage image, QString strNow)
{
//    Q_LOG4CPLUS_INFO(QString("processImage() threadId: %1").arg(quintptr(QThread::currentThreadId())))

    //非检测模式下，模拟视频流显示图像
    if (m_bInTestMode == false)//m_bInTestMode 是否在检测模式
    {
//        Q_LOG4CPLUS_INFO(QString("processImage() start: showImage %1").arg(getDeviceSN()))
        showImage(image, getDeviceSN());
//        Q_LOG4CPLUS_INFO(QString("processImage() end: showImage %1").arg(getDeviceSN()))
    }
    //检测模式
    else
    {
        //根据socket命令保存ic图片
        if (m_bSaveICImageFromSocket)
        {
            Q_LOG4CPLUS_INFO("processImage() start: SaveICImageFromSocket")
         //这个函数有信号
            saveImageForAnnotationInTestModeCallback(image, "ic", strNow);
            m_bSaveICImageFromSocket = false;
            Q_LOG4CPLUS_INFO("processImage() end: SaveICImageFromSocket")
        }
        //根据socket命令保存heat图片
        else if (m_bSaveHeatImageFromSocket)
        {
            Q_LOG4CPLUS_INFO("processImage() start: SaveHeatImageFromSocket")
            //有信号
            saveImageForAnnotationInTestModeCallback(image, "heat", strNow);
            m_bSaveHeatImageFromSocket = false;
            Q_LOG4CPLUS_INFO("processImage() end: SaveHeatImageFromSocket")
        }
        //根据socket命令检测ic
        else if (m_bDetectICFromSocket)
        {
            Q_LOG4CPLUS_INFO("processImage() start: DetectICFromSocket")
            //有信号
            detectInTestModeCallback(image, "ic", strNow);
            m_bDetectICFromSocket = false;
            Q_LOG4CPLUS_INFO("processImage() end: DetectICFromSocket")
        }
        //根据socket命令检测heat
        else if (m_bDetectHeatFromSocket)
        {
            Q_LOG4CPLUS_INFO("processImage() start: DetectHeatFromSocket")
            //有信号
            detectInTestModeCallback(image, "heat", strNow);
            m_bDetectHeatFromSocket = false;
            Q_LOG4CPLUS_INFO("processImage() end: DetectHeatFromSocket")
        }
    }//检测模式
}

//打开装置需要的一些参数和返回值
//------------------------------------------------------------
/**
\brief   Open Device
\param   strDeviceSN   [in]         设备序列号
\param   pBitmap       [in]         窗口句柄
\param   nDeviceIndex  [in]         设备序号
\return  void
*/
//------------------------------------------------------------
void CDeviceProcess::OpenDevice( gxstring strDeviceSN, CGXBitmap *pBitmap, int nDeviceIndex ) {
    bool bIsDeviceOpen = false;      ///< 判断设备是否已打开标志
    bool bIsStreamOpen = false;      ///< 判断设备流是否已打开标志

    try
    {
        if (m_bIsOpen || pBitmap == nullptr || strDeviceSN == "")
        {
            throw std::exception("invalid argument!");
        }

        m_strDeviceSN = strDeviceSN;
        m_nDeviceIndex = nDeviceIndex + 1;
        m_strSavePath = QString("./images/cam%1/").arg(m_nDeviceIndex);
        //打开设备
        m_objDevicePtr = IGXFactory::GetInstance().OpenDeviceBySN(strDeviceSN, GX_ACCESS_EXCLUSIVE);
        bIsDeviceOpen = true;
        //获取属性控制器
        m_objFeatureControlPtr = m_objDevicePtr->GetRemoteFeatureControl();
        //设置采集模式为连续采集
        m_objFeatureControlPtr->GetEnumFeature("AcquisitionMode")->SetValue("Continuous");
        //设置触发模式为开
//        m_objFeatureControlPtr->GetEnumFeature("TriggerMode")->SetValue("On"); //modified, 原来为off
        m_objFeatureControlPtr->GetEnumFeature("TriggerMode")->SetValue("Off"); //modified
        //设置触发源为软触发
        m_objFeatureControlPtr->GetEnumFeature("TriggerSource")->SetValue("Software"); // 20230309 newly add
        //获取设备流个数
        int nCount = m_objDevicePtr->GetStreamCount();

        if (nCount > 0)
        {
            m_objStreamPtr = m_objDevicePtr->OpenStream(0);
            m_objStreamFeatureControlPtr = m_objStreamPtr->GetFeatureControl();
            bIsStreamOpen = true;
        }
        else
        {
            throw std::exception("未发现设备流!");
        }

        // 建议用户在打开网络相机之后，根据当前网络环境设置相机的流通道包长值，
        // 以提高网络相机的采集性能,设置方法参考以下代码。
        GX_DEVICE_CLASS_LIST objDeviceClass = m_objDevicePtr->GetDeviceInfo().GetDeviceClass();

        if (GX_DEVICE_CLASS_GEV == objDeviceClass)
        {
            // 判断设备是否支持流通道数据包功能
            if (true == m_objFeatureControlPtr->IsImplemented("GevSCPSPacketSize"))
            {
                // 获取当前网络环境的最优包长值
                int nPacketSize = m_objStreamPtr->GetOptimalPacketSize();
                // 将最优包长值设置为当前设备的流通道包长值
                m_objFeatureControlPtr->GetIntFeature("GevSCPSPacketSize")->SetValue(nPacketSize);
            }
        }

        m_bIsOpen = true;
        m_pBitmap = pBitmap;
        m_pBitmap->addDevicePtr(m_objDevicePtr);
    }
    catch (CGalaxyException &e)
    {
        //判断设备流是否已打开
        if (bIsStreamOpen)
        {
            m_objStreamPtr->Close();
        }

        //判断设备是否已打开
        if (bIsDeviceOpen)
        {
            m_objDevicePtr->Close();
        }

        m_bIsOpen = false;
        throw e;
    }
    catch (std::exception &e)
    {
        //判断设备流是否已打开
        if (bIsStreamOpen)
        {
            m_objStreamPtr->Close();
        }

        //判断设备是否已打开
        if (bIsDeviceOpen)
        {
            m_objDevicePtr->Close();
        }

        m_bIsOpen = false;
        throw e;
    }
}
//------------------------------------------------------------
/**
\brief   Close Device

\return  void
*/
//------------------------------------------------------------
void CDeviceProcess::CloseDevice() {
    if ( !m_bIsOpen ) {
        return;
    }

    try {
        //判断是否停止采集
        if ( m_bIsSnap ) {
            //发送停采命令
            m_objFeatureControlPtr->GetCommandFeature( "AcquisitionStop" )->Execute();
            //关闭流层采集
            m_objStreamPtr->StopGrab();
            //注销回调函数
            m_objStreamPtr->UnregisterCaptureCallback();
        }
    }
    catch ( CGalaxyException ) {
        //do noting
    }

    try {
        //关闭流对象
        m_objStreamPtr->Close();
    }
    catch ( CGalaxyException ) {
        //do noting
    }

    try {
        //关闭设备
        m_objDevicePtr->Close();
    }
    catch ( CGalaxyException ) {
        //do noting
    }

    m_bIsSnap = false;
    m_bIsOpen = false;
    m_bIsOpenAndSnap = false;
}
//------------------------------------------------------------
/**
\brief   Start Snap

\return  void
*/
//------------------------------------------------------------
void CDeviceProcess::StartSnap()//开始采集
{
    //判断设备是否已打开
    if (!m_bIsOpen)
    {
        return;
    }
    //设置采集 buffer个数
    m_objStreamPtr->SetAcqusitionBufferNumber(1); //newly add
    //设置buffer处理模式
//    m_objFeatureControlPtr->GetEnumFeature("StreamBufferHandlingMode")->SetValue("NewestOnly"); //newly add

    try
    {
        //注册回调函数
        m_objStreamPtr->RegisterCaptureCallback(m_pCSampleCaptureEventHandler, this);
    }
    catch (CGalaxyException &e)
    {
        throw e;
    }

    try
    {
        //开启流层采集
        m_objStreamPtr->StartGrab();
    }
    catch (CGalaxyException &e)
    {
        //注销回调函数
        m_objStreamPtr->UnregisterCaptureCallback();
        throw e;
    }

    try
    {
        //发送开采命令
        m_objFeatureControlPtr->GetCommandFeature("AcquisitionStart")->Execute();
        m_bIsSnap = true;
        m_bIsOpenAndSnap = true;
    }
    catch (CGalaxyException &e)
    {
        //关闭流层采集
        m_objStreamPtr->StopGrab();
        //注销回调函数
        m_objStreamPtr->UnregisterCaptureCallback();
        throw e;
    }
    catch (std::exception &e)
    {
        //关闭流层采集
        m_objStreamPtr->StopGrab();
        //注销回调函数
        m_objStreamPtr->UnregisterCaptureCallback();
        throw e;
    }
}

//------------------------------------------------------------
/**
\brief   Stop Snap

\return  void
*/
//------------------------------------------------------------
void CDeviceProcess::StopSnap()
{
    //判断设备是否已打开
    if ( !m_bIsOpen || !m_bIsSnap ) {
        return;
    }

    try {
        //发送停采命令
        m_objFeatureControlPtr->GetCommandFeature( "AcquisitionStop" )->Execute();
        //关闭流层采集
        m_objStreamPtr->StopGrab();
        //注销回调函数
        m_objStreamPtr->UnregisterCaptureCallback();
        m_bIsSnap = false;
    }
    catch ( CGalaxyException &e ) {
        throw e;
    }
    catch ( std::exception &e ) {
        throw e;
    }
}

void CDeviceProcess::setBitmap(CGXBitmap *pBitmap)
{
    m_pBitmap = pBitmap;
}

//------------------------------------------------------------
/**
\brief   Get Device Open Flag

\return  void
*/
//------------------------------------------------------------
bool CDeviceProcess::IsOpen() {
    return m_bIsOpen;
}

//------------------------------------------------------------
/**
\brief   Get Device Snap Flag

\return  void
*/
//------------------------------------------------------------
bool CDeviceProcess::IsSnap() {
    return m_bIsSnap;
}

bool CDeviceProcess::IsOpenAndSnap()
{
    return m_bIsOpenAndSnap;
}

//------------------------------------------------------------
/**
\brief   Set Show frame Frequency Flag

\return  void
*/
//------------------------------------------------------------
void CDeviceProcess::SetShowSN( bool bIsShowFps ) {
    m_bShowDeviceSN = bIsShowFps;
}

void CDeviceProcess::setShowImage(bool bShowImage)
{
    m_bShowImage = bShowImage;
}

void CDeviceProcess::setShowFrameInterval(int nShowFrameInterval)
{
    m_nShowFrameInterval = nShowFrameInterval;
    nTmpShowFrameInterval = nShowFrameInterval;
}

//------------------------------------------------------------
/**
\brief   Set save frame Interval

\return  void
*/
//------------------------------------------------------------
void CDeviceProcess::setSaveFrameInterval(int nSaveFrameInterval)
{
    m_nSaveFrameInterval = nSaveFrameInterval;
    nTmpSaveFrameInterval = nSaveFrameInterval;
}

void CDeviceProcess::setSaveFileNameListSize(int nSaveFileNameListSize)
{
    m_nSaveFileNameListSize = nSaveFileNameListSize;
}

//------------------------------------------------------------
/**
\brief   Get Show frame Frequency Flag

\return  void
*/
//------------------------------------------------------------
bool CDeviceProcess::GetShowSN() {
    return m_bShowDeviceSN;
}

//------------------------------------------------------------
/**
\brief   Refresh Device SN

\return  void
*/
//------------------------------------------------------------
void CDeviceProcess::RefreshDeviceSN() {
    sprintf_s( m_pRecordDeviceSN, 1024,
               "No:%d SN:%s",
               m_nDeviceIndex,
               m_strDeviceSN.c_str() );
}

//------------------------------------------------------------
/**
\brief   Show Image
\param   objImageDataPointer [in]       图像信息

\return  void
*/
//------------------------------------------------------------
void CDeviceProcess::__ShowPicture(CImageDataPointer &objImageDataPointer)
{
    if (m_bShowDeviceSN)
    {
        //显示图像和相机型号
        m_pBitmap->Show(objImageDataPointer, m_pRecordDeviceSN);
    }
    else
    {
        //显示图像
        m_pBitmap->Show(objImageDataPointer);
    }
}

void CDeviceProcess::showImage(const QImage &image, QString strDeviceSN)
{
    if (m_bShowDeviceSN)
    {
        //显示图像和相机型号
        m_pBitmap->showImage(image, strDeviceSN);
    }
    else
    {
        //显示图像
        m_pBitmap->showImage(image, "");
//        m_pBitmap->Show(image);
    }
}

void CDeviceProcess::setDeviceIndex(int index)
{
    m_nDeviceIndex = index + 1;
    m_strSavePath = QString("./images/cam%1/").arg(m_nDeviceIndex);
}

bool CDeviceProcess::needCreateImage()
{
    return m_bSaveICImageFromSocket || m_bSaveHeatImageFromSocket || m_bDetectICFromSocket || m_bDetectHeatFromSocket ||
           (m_bShowImage && m_nShowFrameInterval > 0 && (nTmpShowFrameInterval >= m_nShowFrameInterval)) ||
            (m_bSaveImage && m_nSaveFrameInterval > 0 && (nTmpSaveFrameInterval >= m_nSaveFrameInterval));
}

int CDeviceProcess::getType(cv::Mat &src)
{
    return utils.getType(src);
}

//----------------------------------------------------------------------------------
/**
\brief   将图像保存成BMP图片
\param   objImageDataPointer    [in]         图像数据指针
\param   qstrImageName          [in]         保存图像的名称
\return  无
*/
//----------------------------------------------------------------------------------
void CDeviceProcess::SavePicture(CImageDataPointer &objImageDataPointer)
{
    NoteTimingStart;
    QString strFuncInfoAndDeviceSN = QString("%1 Device:%2 ").arg(__FUNCSIG__, QString(m_strDeviceSN));
    //    Q_LOG4CPLUS_INFO(strFuncInfoAndDeviceSN + "Start to save an image.");
    try
    {
        QString strFileName = ""; ///< 图像保存路径名称
        QString strFilePath = ""; ///< 文件路径
        strFilePath = m_strSavePath;

        //采用Qt的方式，创建保存图像的文件夹
        QString path = m_strSavePath;
        QDir dir(path);
        if (!dir.exists())
        {
            bool flag = QDir().mkdir(path);
            if (flag)
            {
                Q_LOG4CPLUS_INFO(strFuncInfoAndDeviceSN + "Successfully make a dir: " + path)
            }
            else
            {
                Q_LOG4CPLUS_ERROR(strFuncInfoAndDeviceSN + "Fail to make a dir: " + path)
                return;
            }
        }

        //若图像保存列表已满，出队删除图片
        if (m_bSaveImage && m_nSaveFileNameListSize && m_qSaveFileNameList.size() >= m_nSaveFileNameListSize)
        {
            QString tempFileName = m_qSaveFileNameList.front();
            m_qSaveFileNameList.pop();
            QFile tempFile(tempFileName);
            if (tempFile.exists())
            {
                if (tempFile.remove())
                {
                    Q_LOG4CPLUS_INFO(strFuncInfoAndDeviceSN + "Successfully delete the oldest image.");
                }
                else
                {
                    Q_LOG4CPLUS_ERROR(strFuncInfoAndDeviceSN + "Fail to delete the oldest image.")
                }
            }
        }

        //获取当前时间为图像保存的默认名称
        QString strNow = QDateTime::currentDateTime().toString("-yyyy-MM-dd-hh-mm-ss-zzz");
        strFileName = strFilePath + m_strDeviceSN.c_str() + strNow + ".bmp";
//        Q_LOG4CPLUS_INFO(strFuncInfoAndDeviceSN + QString::fromStdString("strFileName: " + strFileName));
        if (m_bSaveImage)
        {
            m_qSaveFileNameList.push(strFileName);
        }
        //保存图像为BMP
//        m_pBitmap->SaveBmp(objImageDataPointer, strFileName);
        //        Q_LOG4CPLUS_INFO(strFuncInfoAndDeviceSN + "Finish saving an image.");
    }
    catch (std::exception &e)
    {
        Q_LOG4CPLUS_ERROR(QString("exception: ") + QString(e.what()));
        //由于存图是在线程回调中实现的，如果存图抛出异常。采集线程将终止，为了避免线程终止，存图将对异常不做处理
        return;
    }
}

//----------------------------------------------------------------------------------
/**
\brief   保存图片，若目录不存在则创建目录
\param   objImageDataPointer    [in]         图像数据指针
\param   qstrImageName          [in]         保存图像的名称
\return  无
*/
//----------------------------------------------------------------------------------
void CDeviceProcess::saveImage(QImage &image, QString &strFileNameWithPath)
{
//    NoteTimingStart;
    QString strFuncInfoAndDeviceSN = QString("%1 Device:%2 ").arg(__FUNCSIG__, QString(m_strDeviceSN));
    try
    {
        //创建保存图像的文件夹
        QString savePath = QString("./images/cam%1").arg(m_nDeviceIndex + 1);
        if (!QDir().exists(savePath))
        {
            bool flag = QDir().mkpath(savePath);
            if (flag)
            {
                Q_LOG4CPLUS_INFO(strFuncInfoAndDeviceSN + "Successfully make a dir: " + savePath)
            }
            else
            {
                Q_LOG4CPLUS_ERROR(strFuncInfoAndDeviceSN + "Fail to make a dir: " + savePath)
                return;
            }
        }

        //若图像保存列表已满，出队删除图片
        if (m_bSaveImage && m_nSaveFileNameListSize && m_qSaveFileNameList.size() >= m_nSaveFileNameListSize)
        {
            QString tempFileName = m_qSaveFileNameList.front();
            m_qSaveFileNameList.pop();
            QFile tempFile(tempFileName);
            if (tempFile.exists())
            {
                if (tempFile.remove())
                {
                    Q_LOG4CPLUS_INFO(strFuncInfoAndDeviceSN + "Successfully delete the oldest image.");
                }
                else
                {
                    Q_LOG4CPLUS_ERROR(strFuncInfoAndDeviceSN + "Fail to delete the oldest image.")
                }
            }
        }

        //保存图像为BMP
        m_pBitmap->saveImage(image, strFileNameWithPath);
        if (m_bSaveImage)
        {
            m_qSaveFileNameList.push(strFileNameWithPath);
        }
    }
    catch (std::exception &e)
    {
        Q_LOG4CPLUS_ERROR(QString("exception: ") + QString(e.what()));
        //由于存图是在线程回调中实现的，如果存图抛出异常。采集线程将终止，为了避免线程终止，存图将对异常不做处理
        return;
    }
}

//检测模式下，保存图像用于标注 没用到
void CDeviceProcess::saveImageForAnnotationInTestMode(const QString &camera)
{
    NoteTimingStart
    QString strFuncInfoAndDeviceSN = QString("%1 Device:%2 ").arg(__FUNCSIG__, QString(m_strDeviceSN));
    //发送软触发命令(在触发模式开启时有效)
    m_objFeatureControlPtr->GetCommandFeature("TriggerSoftware")->Execute(); // newly add
    m_objStreamPtr->FlushQueue(); //清空采集队列中的缓存图像，保证下面采集的图片是最新的
    CImageDataPointer objImageDataPtr = m_objStreamPtr->GetImage(500); //500
    if (objImageDataPtr->GetStatus() != GX_FRAME_STATUS_SUCCESS)
    {
        qDebug() << "objImageDataPtr->GetStatus() != GX_FRAME_STATUS_SUCCESS";
        Q_LOG4CPLUS_ERROR("objImageDataPtr->GetStatus() != GX_FRAME_STATUS_SUCCESS")
    }
    //若采图成功而且是完整帧，则保存图像，并显示到显示区
    else if (objImageDataPtr->GetStatus() == GX_FRAME_STATUS_SUCCESS)
    {
        uchar *pBuffer = (uchar *)objImageDataPtr->GetBuffer();
        QImage image = QImage(pBuffer,
                              objImageDataPtr->GetWidth(),
                              objImageDataPtr->GetHeight(),
                              QImage::Format_Grayscale8);
        //路径
        QString strNow = QDateTime::currentDateTime().toString("-yyyyMMdd-hhmmss-zzz");
        QString strFileNameWithPath = QString("./images/cam%1/").arg(m_nDeviceIndex + 1) +
                                      QString("cam%1").arg(m_nDeviceIndex + 1) +
                                      strNow + ".bmp";


        saveImage(image, strFileNameWithPath);
        QString sendMsg = QString("TakePhoto %1 1").arg(camera);
        emit send_to_mainwind(sendMsg);
        Q_LOG4CPLUS_INFO(strFuncInfoAndDeviceSN + "socket write: " + sendMsg)
    }
    else
    {
        Q_LOG4CPLUS_WARN(strFuncInfoAndDeviceSN + "Fail to get a single image.")
    }
}

void CDeviceProcess::saveImageForAnnotationInTestModeCallback(QImage &img, const QString &camera, const QString &strNow)
{
    QString strFuncInfoAndDeviceSN = QString("%1 Device:%2 ").arg(__FUNCSIG__, QString(m_strDeviceSN));

    //路径
//    QString strNow = QDateTime::currentDateTime().toString("-yyyyMMdd-hhmmss-zzz");
    QString strFileNameWithPath = QString("./images/cam%1/").arg(m_nDeviceIndex) +
                                  QString("cam%1-").arg(m_nDeviceIndex) +
                                  strNow + ".bmp";


    saveImage(img, strFileNameWithPath);
    QString sendMsg = QString("TakePhoto %1 1").arg(camera);
    emit send_to_mainwind(sendMsg);
    Q_LOG4CPLUS_INFO(strFuncInfoAndDeviceSN + "socket write: " + sendMsg)
}

void CDeviceProcess::triggerSoftware()
{
    //获取一个命令型控制器
    m_objFeatureControlPtr->GetCommandFeature("TriggerSoftware")->Execute(); // newly add
    Q_LOG4CPLUS_INFO(QString("triggerSoftware()"))
}

void CDeviceProcess::showRecursivelyNotInTestMode()
{
    while (!m_bInTestMode)
    {
        Q_LOG4CPLUS_INFO(QString("showRecursivelyNotInTestMode()"))
        m_objFeatureControlPtr->GetCommandFeature("TriggerSoftware")->Execute(); // newly add
        m_objStreamPtr->FlushQueue(); //清空采集队列中的缓存图像，保证下面采集的图片是最新的
        CImageDataPointer objImageDataPtr = m_objStreamPtr->GetImage(500); //500
        if (objImageDataPtr->GetStatus() != GX_FRAME_STATUS_SUCCESS)
        {
            qDebug() << "objImageDataPtr->GetStatus() != GX_FRAME_STATUS_SUCCESS";
            Q_LOG4CPLUS_ERROR("objImageDataPtr->GetStatus() != GX_FRAME_STATUS_SUCCESS")
        }
        //若采图成功而且是完整帧，则保存图像，并显示到显示区
        else if (objImageDataPtr->GetStatus() == GX_FRAME_STATUS_SUCCESS)
        {
            uchar *pBuffer = (uchar *)objImageDataPtr->GetBuffer();
            QImage image = QImage(pBuffer,
                                  objImageDataPtr->GetWidth(),
                                  objImageDataPtr->GetHeight(),
                                  QImage::Format_Grayscale8);
            QString strDeviceSN = QString(m_strDeviceSN);
            Q_LOG4CPLUS_INFO(QString("showRecursivelyNotInTestMode() start: showImage"))
            showImage(image, strDeviceSN);
        }
    }
}

//检测 没用到
void CDeviceProcess::detectInTestMode(const QString &camera)
{
    Q_LOG4CPLUS_INFO(QString("detectInTestMode() start"))
//    NoteTimingStart
    QString strFuncInfoAndDeviceSN = QString("%1 Device:%2 ").arg(__FUNCSIG__, QString(m_strDeviceSN));
    //发送软触发命令(在触发模式开启时有效)
    m_objFeatureControlPtr->GetCommandFeature("TriggerSoftware")->Execute(); // newly add
    //开始拍照
    Q_LOG4CPLUS_INFO(QString("detectInTestMode() start: take photo"))
    m_objStreamPtr->FlushQueue(); //清空采集队列中的缓存图像，保证下面采集的图片是最新的
    CImageDataPointer objImageDataPtr = m_objStreamPtr->GetImage(500); //500
    if (objImageDataPtr->GetStatus() != GX_FRAME_STATUS_SUCCESS)
    {
        qDebug() << "objImageDataPtr->GetStatus() != GX_FRAME_STATUS_SUCCESS";
        Q_LOG4CPLUS_ERROR("objImageDataPtr->GetStatus() != GX_FRAME_STATUS_SUCCESS")
    }
    //若采图成功而且是完整帧，则保存图像，并显示到显示区
    else if (objImageDataPtr->GetStatus() == GX_FRAME_STATUS_SUCCESS)
    {
        uchar *pBuffer = (uchar *)objImageDataPtr->GetBuffer();
        QImage image = QImage(pBuffer,
                              objImageDataPtr->GetWidth(),
                              objImageDataPtr->GetHeight(),
                              QImage::Format_Grayscale8);
        QString strNow = QDateTime::currentDateTime().toString("-yyyyMMdd-hhmmss-zzz"); // 获取图片的时间戳
        //---------------------------------------
        // 开始检测
        Q_LOG4CPLUS_INFO(QString("detectInTestMode() start: Detect %1").arg(camera))
        int result_code = -1;
        QImage image_result;
        if (camera == "ic")
        {
            const auto &[code, image_res] = ICWindow::dealWithQImage(image); //结构化绑定，获取tuple，变
            result_code = code;
            image_result = image_res.copy();
        }
        else if (camera == "heat")
        {
            const auto &[code, image_res] = FilmStripWindow::dealWithQImage(image); //结构化绑定，获取tuple，变
            result_code = code;
            image_result = image_res.copy();
        }

        if (image_result.isNull())
        {
            std::cerr << "image_result.isNull()" << std::endl;
            return;
        }
        Q_LOG4CPLUS_INFO(QString("detectInTestMode() end: Detect %1").arg(camera))
        QString strDeviceSN = "";
        showImage(image_result, strDeviceSN);

        //先发送socket，再保存结果图，后续是否需要调整再说
        QString sendMsg = QString("Test %1 %2").arg(camera).arg(QString::number(result_code));
        emit send_to_mainwind(sendMsg);
        Q_LOG4CPLUS_INFO(strFuncInfoAndDeviceSN + "socket write: " + sendMsg)
        //保存原图和结果图
        Q_LOG4CPLUS_INFO(QString("detectInTestMode() start: save %1 result").arg(camera))
        QString saveDir = QString("./images/%1/").arg(camera);
        if (!QDir().exists(saveDir))
        {
            QDir().mkpath(saveDir);
        }
        QString savePath_src = saveDir + QString("cam%1").arg(m_nDeviceIndex + 1) + strNow + + ".bmp";
        QString savePath_result = saveDir + QString("cam%1").arg(m_nDeviceIndex + 1) + strNow + "-result.bmp";
        image.save(savePath_src);
        image_result.save(savePath_result);
    }
    Q_LOG4CPLUS_INFO(QString("detectInTestMode() end"))
}

//这个函数有信号
void CDeviceProcess::detectInTestModeCallback(QImage &image, const QString &camera, const QString &strNow)
{
    Q_LOG4CPLUS_INFO(QString("detectInTestModeCallback() start"))
    QString strFuncInfoAndDeviceSN = QString("%1 Device:%2 ").arg(__FUNCSIG__, getDeviceSN());

    //---------------------------------------
    // 开始检测
    Q_LOG4CPLUS_INFO(QString("detectInTestMode() start: Detect %1").arg(camera))
    int result_code = -1;
    QImage image_result;
    if (camera == "ic")
    {
        const auto &[code, image_res] = ICWindow::dealWithQImage(image); //结构化绑定，获取tuple
        result_code = code;
        image_result = image_res.copy();
    }
    else if (camera == "heat")
    {
        const auto &[code, image_res] = FilmStripWindow::dealWithQImage(image); //结构化绑定，获取tuple
        result_code = code;
        image_result = image_res.copy();
    }

    if (image_result.isNull())
    {
        std::cerr << "image_result.isNull()" << std::endl;
        return;
    }
    Q_LOG4CPLUS_INFO(QString("detectInTestMode() end: Detect %1").arg(camera))

    //显示结果
    showImage(image_result, getDeviceSN());

    //保存原图和结果图
    Q_LOG4CPLUS_INFO(QString("detectInTestMode() start: save %1 result").arg(camera))
    QString saveDir = QString("./images/%1/").arg(camera);
    if (!QDir().exists(saveDir))
    {
        QDir().mkpath(saveDir);
    }
    QString savePath_src = saveDir + QString("cam%1-").arg(m_nDeviceIndex) + strNow + + ".bmp";
    QString savePath_result = saveDir + QString("cam%1-").arg(m_nDeviceIndex) + strNow + "-result.bmp";
    image.save(savePath_src);
    image_result.save(savePath_result);
    Q_LOG4CPLUS_INFO(QString("detectInTestMode() end: save %1 result").arg(camera))

    //发送socket
    QString sendMsg = QString("Test %1 %2").arg(camera).arg(QString::number(result_code));

    emit send_to_mainwind(sendMsg);
    Q_LOG4CPLUS_INFO(strFuncInfoAndDeviceSN + "socket write: " + sendMsg)

    Q_LOG4CPLUS_INFO(QString("detectInTestMode() end"))
}

//----------------------------------------------------------------------------------
/**
\brief   设置保存图像标识
\param   bSaveImage             [in]         保存图像标识
\return  无
*/
//----------------------------------------------------------------------------------
void CDeviceProcess::setSaveImage(bool bSaveImage)
{
    m_bSaveImage = bSaveImage;
}

//----------------------------------------------------------------------------------
/**
\brief   设置根据socket命令保存图像标识
\param   bSaveImageFromSocket    [in]         保存图像标识
\return  无
*/
//----------------------------------------------------------------------------------
void CDeviceProcess::setSaveICImageFromSocket(bool bSaveImageFromSocket)
{
    m_bSaveICImageFromSocket = bSaveImageFromSocket;
}

void CDeviceProcess::setSaveHeatImageFromSocket(bool bSaveImageFromSocket)
{
    m_bSaveHeatImageFromSocket = bSaveImageFromSocket;
}

void CDeviceProcess::setDetectICFromSocket(bool bDetectICFFromSocket)
{
    m_bDetectICFromSocket = bDetectICFFromSocket;
}

void CDeviceProcess::setDetectHeatFromSocket(bool bDetectHeatFromSocket)
{
    m_bDetectHeatFromSocket = bDetectHeatFromSocket;
}

//根据传入的图像数据指针objImageDataPointer即GXSmartPtr<IImageData>，创建一个QImage类型的对象image，并通过信号imageReady将其传递出去。
//在函数执行过程中，如果出现CGalaxyException或std::exception异常，则会打印日志记录下来。
void CSampleCaptureEventHandler::DoOnImageCaptured(CImageDataPointer &objImageDataPointer, void *pUserParam)
{
//    Q_LOG4CPLUS_INFO(QString("DoOnImageCaptured() threadId: %1").arg(quintptr(QThread::currentThreadId())))
    try
    {
        CDeviceProcess *pDeviceProcess = (CDeviceProcess *)pUserParam;
        QString strFuncInfoAndDeviceSN = QString("%1 Device:%2 ").arg(__FUNCSIG__, pDeviceProcess->getDeviceSN());
        if (objImageDataPointer->GetStatus() != GX_FRAME_STATUS_SUCCESS)
        {
            Q_LOG4CPLUS_ERROR(strFuncInfoAndDeviceSN + "objImageDataPtr->GetStatus() != GX_FRAME_STATUS_SUCCESS");
            throw std::runtime_error("objImageDataPointer is null");
        }
        else if (objImageDataPointer->GetStatus() == GX_FRAME_STATUS_SUCCESS)
        {
//            Q_LOG4CPLUS_INFO("DoOnImageCaptured() start: create image")
            uchar *pBuffer = (uchar *)objImageDataPointer->GetBuffer();
            QString strNow = QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss-zzz");
            QString strFileNameWithPath = QString("./images/cam%1/").arg(pDeviceProcess->getDeviceIndex() + 1) +
                                          QString("cam%1-").arg(pDeviceProcess->getDeviceIndex() + 1) +
                                          strNow + ".bmp";
            QImage image = QImage(pBuffer,
                                  objImageDataPointer->GetWidth(),
                                  objImageDataPointer->GetHeight(),
                                  QImage::Format_Grayscale8).copy(); //不知pBuffer如何管理内存，为了安全起见，还是copy吧

            // 图像旋转180°
            QTransform matrix;
            matrix.rotate(180);
            image = image.transformed(matrix);

            emit imageReady(image, strNow);
//            Q_LOG4CPLUS_INFO("DoOnImageCaptured() end: create image")
        }
    }
    catch (CGalaxyException &e)
    {
        QString logContent = QString("%1 %2 %3").arg(__FUNCSIG__, "Catch a CGalaxyException: ", e.what());
        Q_LOG4CPLUS_ERROR(logContent)
    }
    catch (std::exception &e)
    {
        QString logContent = QString("%1 %2 %3").arg(__FUNCSIG__, "Catch a std::exception: ", e.what());
        Q_LOG4CPLUS_ERROR(logContent)
    }
}
