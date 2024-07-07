﻿#include "cgxbitmap.h"
#include "ui_cgxbitmap.h"
//---------------------------------------------------------------------------------
/**
\brief   构造函数
\param   objCGXDevicePointer 图像设备指针
\param   parent 窗体指针
\return  无
*/
//----------------------------------------------------------------------------------
CGXBitmap::CGXBitmap( QWidget *parent )
    : QWidget( parent )
    , ui(new Ui::CGXBitmap)
    , m_bIsColor( false )
    , m_nImageHeight( 0 )
    , m_nImageWidth( 0 )
    , m_pBmpInfo( nullptr )
    , m_imageWidget(new DisplayImageWidget)
    , m_pImage ( nullptr )
    , m_pImageBuffer( nullptr ) {
    ui->setupUi(this);

    //初始化绘图框
//    m_pLabel = new QLabel(this);
//    m_pLabel->move(0, 0);
//    m_pLabel->resize(402, 303); //采集图片分辨率为 4024x3036，缩放以保证按正确比例显示图片
//    m_pLabel->resize(244, 204); //测试图片分辨率为 2448x2048，缩放以保证按正确比例显示图片

    m_pFps = new QLabel(this);
    m_pFps->move(0, 0);
    m_pFps->resize(300, 30);
    m_pFps->setStyleSheet("color:red;");
    m_pFps->setText("0");

    //自己实现的widget显示图片
//    ui->widget->move(0, 0);
//    ui->widget->resize(802, 606);
//    ui->widget->resize(244, 204);
}

void CGXBitmap::addDevicePtr( CGXDevicePointer &objCGXDevicePointer ) {
    if ( objCGXDevicePointer.IsNull() ) {
        Q_LOG4CPLUS_ERROR("Argument is error");
        throw std::runtime_error( "Argument is error" );
    }

    memset(m_chBmpBuf, 0, sizeof(m_chBmpBuf));
    gxstring strValue = "";
    //获得图像宽度、高度等
    m_nImageWidth = ( int64_t )objCGXDevicePointer->GetRemoteFeatureControl()->GetIntFeature( "Width" )->GetValue();
    m_nImageHeight = ( int64_t )objCGXDevicePointer->GetRemoteFeatureControl()->GetIntFeature( "Height" )->GetValue();
    //获取是否为彩色相机
    __IsSupportColor( objCGXDevicePointer, m_bIsColor );

    if ( m_bIsColor ) {
        __ColorPrepareForShowImg();
    }
    else {
        __MonoPrepareForShowImg();
    }
}
//---------------------------------------------------------------------------------
/**
\brief   析构函数

\return  无
*/
//----------------------------------------------------------------------------------
CGXBitmap::~CGXBitmap() {
}

//----------------------------------------------------------------------------------
/**
\brief     判断PixelFormat是否为8位
\param     emPixelFormatEntry 图像数据格式
\return    true为8为数据，false为非8位数据
*/
//----------------------------------------------------------------------------------
bool CGXBitmap::__IsPixelFormat8( GX_PIXEL_FORMAT_ENTRY emPixelFormatEntry ) {
    bool bIsPixelFormat8 = false;
    const unsigned  PIXEL_FORMATE_BIT = 0x00FF0000;  ///<用于与当前的数据格式进行与运算得到当前的数据位数
    unsigned uiPixelFormatEntry = ( unsigned )emPixelFormatEntry;

    if ( ( uiPixelFormatEntry & PIXEL_FORMATE_BIT ) == GX_PIXEL_8BIT ) {
        bIsPixelFormat8 = true;
    }

    return bIsPixelFormat8;
}

//----------------------------------------------------------------------------------
/**
\brief     通过GX_PIXEL_FORMAT_ENTRY获取最优Bit位
\param     emPixelFormatEntry 图像数据格式
\return    最优Bit位
*/
//----------------------------------------------------------------------------------
GX_VALID_BIT_LIST CGXBitmap::GetBestValudBit( GX_PIXEL_FORMAT_ENTRY emPixelFormatEntry ) {
    GX_VALID_BIT_LIST emValidBits = GX_BIT_0_7;

    switch ( emPixelFormatEntry ) {
    case GX_PIXEL_FORMAT_MONO8:
    case GX_PIXEL_FORMAT_BAYER_GR8:
    case GX_PIXEL_FORMAT_BAYER_RG8:
    case GX_PIXEL_FORMAT_BAYER_GB8:
    case GX_PIXEL_FORMAT_BAYER_BG8: {
        emValidBits = GX_BIT_0_7;
        break;
    }

    case GX_PIXEL_FORMAT_MONO10:
    case GX_PIXEL_FORMAT_BAYER_GR10:
    case GX_PIXEL_FORMAT_BAYER_RG10:
    case GX_PIXEL_FORMAT_BAYER_GB10:
    case GX_PIXEL_FORMAT_BAYER_BG10: {
        emValidBits = GX_BIT_2_9;
        break;
    }

    case GX_PIXEL_FORMAT_MONO12:
    case GX_PIXEL_FORMAT_BAYER_GR12:
    case GX_PIXEL_FORMAT_BAYER_RG12:
    case GX_PIXEL_FORMAT_BAYER_GB12:
    case GX_PIXEL_FORMAT_BAYER_BG12: {
        emValidBits = GX_BIT_4_11;
        break;
    }

    case GX_PIXEL_FORMAT_MONO14: {
        //暂时没有这样的数据格式待升级
        break;
    }

    case GX_PIXEL_FORMAT_MONO16:
    case GX_PIXEL_FORMAT_BAYER_GR16:
    case GX_PIXEL_FORMAT_BAYER_RG16:
    case GX_PIXEL_FORMAT_BAYER_GB16:
    case GX_PIXEL_FORMAT_BAYER_BG16: {
        //暂时没有这样的数据格式待升级
        break;
    }

    default:
        break;
    }

    return emValidBits;
}

//---------------------------------------------------------------------------------
/**
\brief   为彩色相机图像显示准备资源

\return  无
*/
//----------------------------------------------------------------------------------
void CGXBitmap::__ColorPrepareForShowImg() {
    //--------------------------------------------------------------------
    //---------------------------初始化bitmap头---------------------------
    m_pBmpInfo = ( BITMAPINFO * )m_chBmpBuf;
    m_pBmpInfo->bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
    m_pBmpInfo->bmiHeader.biWidth = ( LONG )m_nImageWidth;
    m_pBmpInfo->bmiHeader.biHeight = ( LONG )m_nImageHeight;
    m_pBmpInfo->bmiHeader.biPlanes = 1;
    m_pBmpInfo->bmiHeader.biBitCount = 24;
    m_pBmpInfo->bmiHeader.biCompression = BI_RGB;
    m_pBmpInfo->bmiHeader.biSizeImage = 0;
    m_pBmpInfo->bmiHeader.biXPelsPerMeter = 0;
    m_pBmpInfo->bmiHeader.biYPelsPerMeter = 0;
    m_pBmpInfo->bmiHeader.biClrUsed = 0;
    m_pBmpInfo->bmiHeader.biClrImportant = 0;
}

//---------------------------------------------------------------------------------
/**
\brief   为黑白相机图像显示准备资源

\return  无
*/
//----------------------------------------------------------------------------------
void CGXBitmap::__MonoPrepareForShowImg() {
    //---------------------------------------------------------------------
    //----------------------初始化bitmap头---------------------------------
    m_pBmpInfo = ( BITMAPINFO * )m_chBmpBuf;
    m_pBmpInfo->bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
    m_pBmpInfo->bmiHeader.biWidth = ( LONG )m_nImageWidth;
    m_pBmpInfo->bmiHeader.biHeight = ( LONG )m_nImageHeight;
    m_pBmpInfo->bmiHeader.biPlanes = 1;
    m_pBmpInfo->bmiHeader.biBitCount = 8; // 黑白图像为8
    m_pBmpInfo->bmiHeader.biCompression = BI_RGB;
    m_pBmpInfo->bmiHeader.biSizeImage = 0;
    m_pBmpInfo->bmiHeader.biXPelsPerMeter = 0;
    m_pBmpInfo->bmiHeader.biYPelsPerMeter = 0;
    m_pBmpInfo->bmiHeader.biClrUsed = 0;
    m_pBmpInfo->bmiHeader.biClrImportant = 0;

    // 黑白图像需要初始化调色板
    for ( int i = 0; i < 256; i++ ) {
        m_pBmpInfo->bmiColors[i].rgbBlue = i;
        m_pBmpInfo->bmiColors[i].rgbGreen = i;
        m_pBmpInfo->bmiColors[i].rgbRed = i;
        m_pBmpInfo->bmiColors[i].rgbReserved = 0;
    }

    //为经过翻转后的图像数据分配空间
    if ( m_pImageBuffer != nullptr ) {
        delete m_pImageBuffer;
        m_pImageBuffer = nullptr;
    }

    m_pImageBuffer = new BYTE[( size_t )( m_nImageWidth * m_nImageHeight )];

    if ( m_pImageBuffer == nullptr ) {
        Q_LOG4CPLUS_ERROR("Fail to allocate memory");
        throw std::runtime_error( "Fail to allocate memory" );
    }
}

//----------------------------------------------------------------------------------
/**
\brief     判断是否兼容
\param     pBmpInfo BITMAPINFO指针
\param     nWidth 图像宽
\param     nHeight 图像高
\return    true为一样，false不一样
*/
//----------------------------------------------------------------------------------
bool CGXBitmap::__IsCompatible( BITMAPINFO *pBmpInfo, uint64_t nWidth, uint64_t nHeight ) {
    if ( pBmpInfo == nullptr
            || pBmpInfo->bmiHeader.biHeight != nHeight
            || pBmpInfo->bmiHeader.biWidth != nWidth
       ) {
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------------
/**
\brief     检查图像是否改变并更新Buffer并为图像显示准备资源
\param     objCImageDataPointer  图像数据对象
\return    无
*/
//----------------------------------------------------------------------------------
void CGXBitmap::__UpdateBitmap( CImageDataPointer &objCImageDataPointer ) {
    if ( !__IsCompatible( m_pBmpInfo, objCImageDataPointer->GetWidth(), objCImageDataPointer->GetHeight() ) ) {
        m_nImageWidth = objCImageDataPointer->GetWidth();
        m_nImageHeight = objCImageDataPointer->GetHeight();

        if ( m_bIsColor ) {
            __ColorPrepareForShowImg();
        }
        else {
            __MonoPrepareForShowImg();
        }
    }
}

//---------------------------------------------------------------------------------
/**
\brief   将m_pBufferRGB中图像显示到界面
\param   pBuffer  图像数据Buffer指针
\return  无
*/
//----------------------------------------------------------------------------------
//void CGXBitmap::__DrawImg( uchar *pBuffer ) {
////    NoteTimingStart;

//    if (m_pImage != NULL)
//    {
//        delete m_pImage;
//        m_pImage = NULL;
//    }

//    //数据转换
//    m_pImage = new QImage(pBuffer, m_nImageWidth, m_nImageHeight, QImage::Format_Indexed8);
//    // 不失真缩放
//    m_pImage->scaled(m_nImageWidth, m_nImageHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation); ///!!!
//    m_pLabel->setScaledContents(true);
//    m_pLabel->setPixmap(QPixmap::fromImage(*m_pImage));
//    m_pFps->setText("");
//}
//---------------------------------------------------------------------------------
/**
\brief   将m_pBuffer中图像显示到界面
\param   pBuffer         图像数据Buffer指针
\param   strDeviceSNFPS  设备帧率序列号
\return  无
*/
//----------------------------------------------------------------------------------
void CGXBitmap::__DrawImg(uchar *pBuffer, const char *strDeviceSNFPS)
{
//    NoteTimingStart;

    QString strFuncInfoAndDeviceSN = QString("%1 %2 ").arg(__FUNCSIG__).arg(strDeviceSNFPS);
//    Q_LOG4CPLUS_INFO(strFuncInfoAndDeviceSN + "Start to show an image.");
//    Q_LOG4CPLUS_INFO( strFuncInfoAndDeviceSN + "address" + QString::number((quintptr)pBuffer) );

    // 不失真缩放
//    if ( !m_bIsColor ) {
//        ui->widget->resetImage( QImage(
//                                               pBuffer,
//                                               m_nImageWidth,
//                                               m_nImageHeight,
//                                               QImage::Format_Indexed8 ).mirrored( true, true ) );
//    }
//    else {
//        ui->widget->resetAndCopyImage( QImage(
//                                               pBuffer,
//                                               m_nImageWidth,
//                                               m_nImageHeight,
//                                               QImage::Format_Indexed8 ) );
//    }
    ui->widget->resetAndCopyImage(QImage(pBuffer, m_nImageWidth, m_nImageHeight, QImage::Format_Grayscale8));
//    ui->widget->resetImage(QImage(pBuffer, m_nImageWidth, m_nImageHeight, QImage::Format_Grayscale8));

    m_pFps->setText(QString::fromUtf8(strDeviceSNFPS));
//    Q_LOG4CPLUS_INFO(strFuncInfoAndDeviceSN + "Finished showing an image.");

    //    NoteTimingStart;
    //    QString strFuncInfoAndDeviceSN = QString("%1 %2 ").arg(__FUNCSIG__, QString(strDeviceSNFPS));
    //    Q_LOG4CPLUS_INFO(strFuncInfoAndDeviceSN + "Start to show an image.");

    //    if (m_pImage != NULL)
    //    {
    //        delete m_pImage;
    //        m_pImage = NULL;
    //    }

    //    //数据转换
    ////    m_pImage = new QImage(pBuffer, m_nImageWidth, m_nImageHeight, QImage::Format_Indexed8);
    //    m_pImage = new QImage(pBuffer, 2448, 2048, QImage::Format_Indexed8);
    //    QString str = QString::fromUtf8(strDeviceSNFPS);
    //    // 不失真缩放
    ////    int width = m_pLabel->width(), height = m_pLabel->height();
    ////    m_pImage->scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    //    //暂时用下面这种方式，上面两行的显示图像比例不正确
    //    m_pImage->scaled(m_nImageWidth, m_nImageHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    //    m_pLabel->setScaledContents(true);
    //    m_pLabel->setPixmap(QPixmap::fromImage(*m_pImage));
    //    m_pFps->setText(str);
    //    Q_LOG4CPLUS_INFO(strFuncInfoAndDeviceSN + "Finished showing an image.");
}

void CGXBitmap::__DrawImg(QImage &image, QString &strDeviceSNFPS)
{
//    NoteTimingStart;
    QString strFuncInfoAndDeviceSN = QString("%1 %2 ").arg(__FUNCSIG__).arg(strDeviceSNFPS);
    ui->widget->resetAndCopyImage(image);
//    ui->widget->resetImage(QImage(pBuffer, m_nImageWidth, m_nImageHeight, QImage::Format_Grayscale8));

    m_pFps->setText(strDeviceSNFPS);
}
//---------------------------------------------------------------------------------
/**
\brief   将m_pBufferRGB中图像显示到界面
\param   pBuffer         图像数据Buffer指针
\param   strDeviceSNFPS  设备帧率序列号
\return  无
*/
//----------------------------------------------------------------------------------
void CGXBitmap::__testDrawImg( uchar *pBuffer, const string &strDeviceSNFPS ) {
    NoteTimingStart;

    if (m_pImage != nullptr)
    {
        delete m_pImage;
        m_pImage = nullptr;
    }
    QString str = QString::fromStdString(strDeviceSNFPS);

    //数据转换
//    m_pImage = new QImage(pBuffer, m_nImageWidth, m_nImageHeight, QImage::Format_Indexed8);
    m_pImage = new QImage(pBuffer, 2448, 2048, QImage::Format_Indexed8);
//    m_pImage = new QImage(m_nImageWidth, m_nImageHeight, QImage::Format_Indexed8);
//    uchar *buffer = m_pImage->bits();
//    for (int i = 0; i < m_nImageHeight; i++)
//    {
//        memcpy(buffer + i * m_nImageWidth, pBuffer + i * m_nImageWidth, m_nImageWidth);
//    }

    ui->widget->resetImage(*m_pImage);
//    QImage img = QImage(pBuffer, m_nImageWidth, m_nImageHeight, QImage::Format_Indexed8);
    m_pFps->setText(str);
}
//----------------------------------------------------------------------------------
/**
\brief     计算宽度所占的字节数
\param     nWidth  图像宽度
\param     bIsColor  是否是彩色相机
\return    图像一行所占的字节数
*/
//----------------------------------------------------------------------------------
int64_t CGXBitmap::__GetStride( int64_t nWidth, bool bIsColor ) {
    return bIsColor ? nWidth * 3 : nWidth;
}

//----------------------------------------------------------------------------------
/**
\brief     用于显示图像
\param     objCImageDataPointer  图像数据对象
\return    无
*/
//----------------------------------------------------------------------------------
//void CGXBitmap::Show( CImageDataPointer &objCImageDataPointer ) {
//    GX_VALID_BIT_LIST emValidBits = GX_BIT_0_7;
//    uchar *pBuffer = NULL;

//    if ( objCImageDataPointer.IsNull() ) {
//        Q_LOG4CPLUS_ERROR("NULL pointer dereferenced" );
//        throw std::runtime_error( "NULL pointer dereferenced" );
//    }

////    NoteTimingStart;
//    //检查图像是否改变并更新Buffer
//    __UpdateBitmap( objCImageDataPointer );
//    emValidBits = GetBestValudBit( objCImageDataPointer->GetPixelFormat() );

//    if ( m_bIsColor ) {
//        pBuffer = ( uchar * )objCImageDataPointer->ConvertToRGB24( emValidBits, GX_RAW2RGB_NEIGHBOUR, true );
//        __DrawImg( pBuffer );
//    }
//    else {
//        if ( __IsPixelFormat8( objCImageDataPointer->GetPixelFormat() ) ) {
//            pBuffer = ( uchar * )objCImageDataPointer->GetBuffer();
//        }
//        else {
//            pBuffer = ( uchar * )objCImageDataPointer->ConvertToRaw8( emValidBits );
//        }

//        __DrawImg( pBuffer );
//    }
//}
//----------------------------------------------------------------------------------
/**
\brief     用于显示图像
\param     objCImageDataPointer  图像数据对象
\param     strDeviceSNFPS        图像帧率序列号
\return    无
*/
//----------------------------------------------------------------------------------
void CGXBitmap::Show(CImageDataPointer &objCImageDataPointer, const char *strDeviceSNFPS)
{
    NoteTimingStart;
    GX_VALID_BIT_LIST emValidBits = GX_BIT_0_7;
    BYTE *pBuffer = nullptr;

    if (objCImageDataPointer.IsNull())
    {
        Q_LOG4CPLUS_ERROR(QString(__FUNCSIG__) + " NULL pointer dereferenced");
        throw std::runtime_error("NULL pointer dereferenced");
    }

    //    NoteTimingStart;
    //检查图像是否改变并更新Buffer
    __UpdateBitmap(objCImageDataPointer);
    emValidBits = GetBestValudBit(objCImageDataPointer->GetPixelFormat());

    if (m_bIsColor)
    {
        pBuffer = (uchar *)objCImageDataPointer->ConvertToRGB24(emValidBits, GX_RAW2RGB_NEIGHBOUR, true);
        __DrawImg(pBuffer, strDeviceSNFPS);
    }
    else
    {
        if (__IsPixelFormat8(objCImageDataPointer->GetPixelFormat()))
        {
            pBuffer = (uchar *)objCImageDataPointer->GetBuffer();
        }
        else
        {
            pBuffer = (uchar *)objCImageDataPointer->ConvertToRaw8(emValidBits);
        }

        //// 黑白相机需要翻转数据后显示
        // for (int i = 0; i < m_nImageHeight; i++)
        //{
        //   memcpy(m_pImageBuffer + i * m_nImageWidth, pBuffer + (m_nImageHeight - i - 1) * m_nImageWidth, (size_t)m_nImageWidth);
        // }
        __DrawImg(pBuffer, strDeviceSNFPS);
    }
}

//----------------------------------------------------------------------------------
void CGXBitmap::showImage(const QImage &image, QString strDeviceSN)
{
//    NoteTimingStart; // 显示耗时1~2ms，比较低
    QString strFuncInfoAndDeviceSN = QString("%1 %2 ").arg(__FUNCSIG__).arg(strDeviceSN);
//    ui->widget->resetAndCopyImage(image);
    ui->widget->resetImage(image);
    m_pFps->setText(strDeviceSN);
}

//----------------------------------------------------------------------------------
/**
\brief     用于图像处理后并显示图像
\param     objCfg  图像处理调节参数对象
\param     objCImageDataPointer  图像数据对象
\return    无
*/
//----------------------------------------------------------------------------------
void CGXBitmap::ShowImageProcess( CImageProcessConfigPointer &objCfg, CImageDataPointer &objCImageDataPointer ) {
    if ( ( objCfg.IsNull() ) || ( objCImageDataPointer.IsNull() ) ) {
        Q_LOG4CPLUS_ERROR("NULL pointer dereferenced" );
        throw std::runtime_error( "NULL pointer dereferenced" );
    }

    //检查图像是否改变并更新Buffer
    __UpdateBitmap( objCImageDataPointer );
    BYTE *pBuffer = ( BYTE * )objCImageDataPointer->ImageProcess( objCfg );

    if ( m_bIsColor ) {
        __DrawImg( pBuffer );
    }
    else {
        // 黑白相机需要翻转数据后显示
//        for ( int i = 0; i < m_nImageHeight; i++ ) {
//            memcpy( m_pImageBuffer + i * m_nImageWidth, pBuffer + ( m_nImageHeight - i - 1 ) * m_nImageWidth, ( size_t )m_nImageWidth );
//        }

        __DrawImg( m_pImageBuffer );
    }
}
//----------------------------------------------------------------------------------
/**
\brief     存储Bmp图像
\param     objCImageDataPointer  图像数据对象
\param     strFilePath  显示图像文件名
\return    无
*/
//----------------------------------------------------------------------------------
void CGXBitmap::SaveBmp( CImageDataPointer &objCImageDataPointer, const std::string strFilePath ) {
    NoteTimingStart;

    QString strFuncInfo = QString("%1 ").arg(__FUNCSIG__);

//    Q_LOG4CPLUS_INFO(strFuncInfo + "Start to save an image.");

    GX_VALID_BIT_LIST emValidBits = GX_BIT_0_7;
    uchar *pBuffer = nullptr;

    if ((objCImageDataPointer.IsNull()) || (strFilePath == ""))
    {
        Q_LOG4CPLUS_ERROR(strFuncInfo + "Argument is error");
        throw std::runtime_error("Argument is error");
    }

    //检查图像是否改变并更新Buffer
    __UpdateBitmap(objCImageDataPointer);
    emValidBits = GetBestValudBit(objCImageDataPointer->GetPixelFormat());

    if (m_bIsColor)
    {
        pBuffer = (uchar *)objCImageDataPointer->ConvertToRGB24(emValidBits, GX_RAW2RGB_NEIGHBOUR, true);
    }
    else
    {
        if (__IsPixelFormat8(objCImageDataPointer->GetPixelFormat()))
        {
            pBuffer = (uchar *)objCImageDataPointer->GetBuffer();
        }
        else
        {
            pBuffer = (uchar *)objCImageDataPointer->ConvertToRaw8(emValidBits);
        }

        // 黑白相机需要翻转数据后显示
//        for (int i = 0; i < m_nImageHeight; i++)
//        {
//            memcpy(m_pImageBuffer + i * m_nImageWidth, pBuffer + (m_nImageHeight - i - 1) * m_nImageWidth, (size_t)m_nImageWidth);
//        }
//        pBuffer = m_pImageBuffer;
    }

//    Q_LOG4CPLUS_INFO(strFuncInfo + "Start to create a bmp file.")

    //以下是用qt方式保存
    // 创建文件
    QImage image(pBuffer, m_nImageWidth, m_nImageHeight, QImage::Format_Grayscale8);
    if (image.isNull())
    { // 图片为空则返回
        Q_LOG4CPLUS_ERROR("The image is Null")
        throw std::runtime_error("The image is Null");
    }

    bool isSaveSucceed = image.save(QString::fromStdString(strFilePath));

    if (isSaveSucceed)
    {
        Q_LOG4CPLUS_INFO(strFuncInfo + QString("Saved an Image: ") + QString::fromStdString(strFilePath))
    }
    else
    {
        Q_LOG4CPLUS_ERROR("The image save is failed")
        throw std::runtime_error("The image save is failed");
    }

//    Q_LOG4CPLUS_INFO(strFuncInfo + "Finish saving an image.");

    /////旧的保存方法
//    DWORD dwImageSize = (DWORD)(__GetStride(m_nImageWidth, m_bIsColor) * m_nImageHeight);
//    BITMAPFILEHEADER stBfh = {0};
//    DWORD dwBytesRead = 0;
//    stBfh.bfType = (WORD)'M' << 8 | 'B'; //定义文件类型
//    stBfh.bfOffBits = m_bIsColor ? sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER )
//                      : sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) + ( 256 * 4 ); //定义文件头大小true为彩色,false为黑白
//    stBfh.bfSize = stBfh.bfOffBits + dwImageSize; //文件大小
//    DWORD dwBitmapInfoHeader = m_bIsColor ? sizeof( BITMAPINFOHEADER )
//                               : sizeof( BITMAPINFOHEADER ) + ( 256 * 4 ); //定义BitmapInfoHeader大小true为彩色,false为黑白

//    std::wstring wstr(strFilePath.begin(), strFilePath.end());

//    //创建文件
//    HANDLE hFile = ::CreateFile( wstr.c_str(),
//                                 GENERIC_WRITE,
//                                 0,
//                                 NULL,
//                                 CREATE_ALWAYS,
//                                 FILE_ATTRIBUTE_NORMAL,
//                                 NULL );

//    if (hFile == INVALID_HANDLE_VALUE)
//    {
//        Q_LOG4CPLUS_ERROR(strFuncInfo + "Handle is invalid");
//        throw std::runtime_error("Handle is invalid");
//    }

//    // 黑白相机需要翻转数据再保存
//    for (int i = 0; i < m_nImageHeight; i++)
//    {
//        memcpy(m_pImageBuffer + i * m_nImageWidth, pBuffer + (m_nImageHeight - i - 1) * m_nImageWidth, (size_t)m_nImageWidth);
//    }
//    pBuffer = m_pImageBuffer;

//    ::WriteFile(hFile, &stBfh, sizeof(BITMAPFILEHEADER), &dwBytesRead, NULL);
//    ::WriteFile(hFile, m_pBmpInfo, dwBitmapInfoHeader, &dwBytesRead, NULL); //黑白和彩色自适应
//    ::WriteFile(hFile, pBuffer, dwImageSize, &dwBytesRead, NULL);
//    CloseHandle(hFile);
//    Q_LOG4CPLUS_INFO(strFuncInfo + QString("Saved an Image: ") + QString::fromStdString(strFilePath))
    /////////////以上是旧的保存方法

//    Q_LOG4CPLUS_INFO(strFuncInfo + "Finish saving an image.");
}

//----------------------------------------------------------------------------------
/**
\brief     保存图像
\param     image  图像数据对象
\param     strFileNameWithPath  带有路径的图像文件名
\return    无
*/
//----------------------------------------------------------------------------------
void CGXBitmap::saveImage( QImage &image, QString &strFileNameWithPath ) {
//    NoteTimingStart; // 耗时 13~20ms，保存图片本身不太耗时，耗时主要体现在CDeviceProcess::saveImage中的队列删除文件
    QString strFuncInfo = QString("%1 ").arg(__FUNCSIG__);

    // 图片为空则返回
    if (image.isNull())
    {
        Q_LOG4CPLUS_ERROR("The image is Null")
        throw std::runtime_error("The image is Null");
    }

    bool isSaveSucceed = image.save(strFileNameWithPath, "bmp");

    if (isSaveSucceed)
    {
        Q_LOG4CPLUS_INFO(strFuncInfo + QString("Saved an Image: ") + strFileNameWithPath)
    }
    else
    {
        Q_LOG4CPLUS_ERROR("The image save is failed")
        throw std::runtime_error("The image save is failed");
    }
}

//----------------------------------------------------------------------------------
/**
\brief     存储Raw图像
\param     objCImageDataPointer  图像数据对象
\param     strFilePath  显示图像文件名
\return    无
*/
//----------------------------------------------------------------------------------
void CGXBitmap::SaveRaw( CImageDataPointer &objCImageDataPointer, const std::string strFilePath ) {
    if ( ( objCImageDataPointer.IsNull() ) || ( strFilePath == "" ) ) {
        Q_LOG4CPLUS_ERROR("Argument is error");
        throw std::runtime_error( "Argument is error" );
    }

    //检查图像是否改变并更新Buffer
    __UpdateBitmap( objCImageDataPointer );
    DWORD   dwImageSize = ( DWORD )objCImageDataPointer->GetPayloadSize(); // 写入文件的长度
    DWORD   dwBytesRead = 0;                // 文件读取的长度
    BYTE *pBuffer = ( BYTE * )objCImageDataPointer->GetBuffer();

//    if ( !m_bIsColor ) {
//        // 黑白相机需要翻转数据后显示
//        for ( int i = 0; i < m_nImageHeight; i++ ) {
//            memcpy( m_pImageBuffer + i * m_nImageWidth, pBuffer + ( m_nImageHeight - i - 1 ) * m_nImageWidth, ( size_t )m_nImageWidth );
//        }

//        pBuffer = m_pImageBuffer;
//    }

    // 创建文件
    QImage image( pBuffer, m_nImageWidth, m_nImageHeight, QImage::Format_Indexed8 );

    if ( !m_bIsColor ) {
        image = image.mirrored( true, true );
    }

    const bool isSaveSucceed =  image.save( QString::fromStdString( strFilePath ) );

    if ( isSaveSucceed == false ) { // 创建失败则返回
        Q_LOG4CPLUS_ERROR( "The image save is failed" );
        throw std::runtime_error( "The image save is failed" );
    }


//    // 创建文件
//    HANDLE hFile = ::CreateFile( stringToLPCWSTR( strFilePath ),
//                                 GENERIC_WRITE,
//                                 FILE_SHARE_READ,
//                                 NULL,
//                                 CREATE_ALWAYS,
//                                 FILE_ATTRIBUTE_NORMAL,
//                                 NULL );

//    if ( hFile == INVALID_HANDLE_VALUE ) { // 创建失败则返回
//        Q_LOG4CPLUS_ERROR("Handle is invalid");
//        throw std::runtime_error( "Handle is invalid" );
//    }
//    else {                               // 保存Raw图像
//        ::WriteFile( hFile, pbuffer, dwImageSize, &dwBytesRead, NULL );
//        CloseHandle( hFile );
//    }
}
//----------------------------------------------------------------------------------
/**
\brief     是否支持彩色
\param     objCGXDevicePointer  [in]    设备句柄
\param     bIsColorFilter       [out]   是否支持彩色

\return
*/
//----------------------------------------------------------------------------------
void CGXBitmap::__IsSupportColor( CGXDevicePointer &objCGXDevicePointer, bool &bIsColorFilter ) {
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    bool      bIsImplemented = false;
    bool      bIsMono = false;
    gxstring  strPixelFormat = "";
    strPixelFormat = objCGXDevicePointer->GetRemoteFeatureControl()->GetEnumFeature( "PixelFormat" )->GetValue();
    strPixelFormat.substr( 0, 4 );

    if ( 0 == memcmp( strPixelFormat.c_str(), "Mono", 4 ) ) {
        bIsMono = true;
    }
    else {
        bIsMono = false;
    }

    bIsImplemented = objCGXDevicePointer->GetRemoteFeatureControl()->IsImplemented( "PixelColorFilter" );

    // 若当前为非黑白且支持PixelColorFilter则为彩色
    if ( ( !bIsMono ) && ( bIsImplemented ) ) {
        bIsColorFilter = true;
    }
    else {
        bIsColorFilter = false;
    }
}
