#ifndef CGXBITMAP_H
#define CGXBITMAP_H
//#pragma once

#include <QWidget>
#include "header.h"
#include "TimeRecorder.h"
#include "DisplayImageWidget.h"

namespace Ui {
    class CGXBitmap;
}

class CGXBitmap : public QWidget {
    Q_OBJECT

public:
    CGXBitmap( QWidget *parent = nullptr );
    ~CGXBitmap();

private:
    CGXBitmap& operator=(const CGXBitmap&);
    CGXBitmap(const CGXBitmap&);

public:

    QImage             *m_pImage;
    QLabel             *m_pFps;
    DisplayImageWidget *m_imageWidget;

    //显示图像
//    void Show( CImageDataPointer &objCImageDataPointer );

    //显示图像及帧率
    void Show( CImageDataPointer &objCImageDataPointer, const char *strDeviceSNFPS = "" );

    //显示图像
    void showImage(const QImage &image, QString strDeviceSN);

    //图像处理后并显示图像
    void ShowImageProcess( CImageProcessConfigPointer &objCfg, CImageDataPointer &objCImageDataPointer );

    //存储Bmp图像
    void SaveBmp(CImageDataPointer &objCImageDataPointer, const string strFilePath );

    //保存图像
    void saveImage( QImage &image, QString &strFileNameWithPath );

    //存储Raw图像
    void SaveRaw(CImageDataPointer &objCImageDataPointer, const string strFilePath );

    //通过GX_PIXEL_FORMAT_ENTRY获取最优Bit位
    GX_VALID_BIT_LIST GetBestValudBit( GX_PIXEL_FORMAT_ENTRY emPixelFormatEntry );

    void addDevicePtr( CGXDevicePointer &objCGXDevicePointer );

    //测试将m_pBuffer中图像和帧率显示到界面
    void __testDrawImg( uchar *pBuffer, const string &strDeviceSNFPS );

    //将m_pBufferRGB中图像和帧率显示到界面
    void __DrawImg( uchar *pBuffer, const char *strDeviceSNFPS = "" );

    void __DrawImg(QImage &image, QString &strDeviceSNFPS);

    //计算宽度所占的字节数
    int64_t __GetStride( int64_t nWidth, bool bIsColor );

    //是否支持彩色
    void __IsSupportColor( CGXDevicePointer &objCGXDevicePointer, bool &bIsColorFilter );

private:
    //判断PixelFormat是否为8位
    bool __IsPixelFormat8( GX_PIXEL_FORMAT_ENTRY emPixelFormatEntry );

    //为彩色相机图像显示准备资源
    void __ColorPrepareForShowImg();

    //为黑白相机图像显示准备资源
    void __MonoPrepareForShowImg();

    //判断是否兼容
    bool __IsCompatible( BITMAPINFO *pBmpInfo, uint64_t nWidth, uint64_t nHeight );

    //更新Bitmap的信息
    void __UpdateBitmap( CImageDataPointer &objCImageDataPointer );

    //将m_pBufferRGB中图像显示到界面
//    void __DrawImg( uchar *pBuffer );

    Ui::CGXBitmap *ui;
    bool               m_bIsColor;                       //是否支持彩色相机
//    bool               m_bShowDeviceSN;                  //是否显示设备序列号
    int64_t            m_nImageHeight;                   //原始图像高
    int64_t            m_nImageWidth;                    //原始图像宽
    BITMAPINFO         *m_pBmpInfo;                      //BITMAPINFO 结构指针，显示图像时使用
    char               m_chBmpBuf[2048];                 //BIMTAPINFO 存储缓冲区，m_pBmpInfo即指向此缓冲区
    uchar              *m_pImageBuffer;                  //保存翻转后的图像用于显示
};
#endif // CGXBITMAP_H
