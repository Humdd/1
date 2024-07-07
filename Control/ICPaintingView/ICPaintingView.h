#ifndef ICPAINTINGVIEW_H
#define ICPAINTINGVIEW_H
#include "stable.h"
//C:/WorkSpace/OpenCVProject/FilmStripDetection/Control/AbstractPaintingView/AbstractPaintingView.h

QT_BEGIN_NAMESPACE
namespace Ui {
    class ICPaintingView;
}// namespace Ui
QT_END_NAMESPACE
class ICGraphicsScene;
//---------------------------------------------------------------------------------
/*!
 *@brief   绘图的视图类（画布类），用于显示图片；“MVD”模式中的V（view），主要功能如下：
 *         （1）设置Scene、设置图片
 *         （2）放大、缩小视图
 *         （3）通过拖拽来改变显示的图片
 */
//----------------------------------------------------------------------------------
class ICPaintingView: public QWidget {
    Q_OBJECT

public:
    ICPaintingView( QWidget *parent = nullptr ) noexcept;
    ~ICPaintingView();
    //---------------------------------------------------------------------------------
    /*!
     *@brief   初始化Scene
     */
    //----------------------------------------------------------------------------------
    void initScene( QGraphicsScene *scene ) noexcept;

    //---------------------------------------------------------------------------------
    /*!
     *@brief   用opencv的Mat来重新设置图片
     */
    //----------------------------------------------------------------------------------
    void setImage( const cv::Mat &_image ) noexcept;

    //---------------------------------------------------------------------------------
    /*!
     *@brief   辅助缩放图像，这是真正缩放图像的函数
     */
    //----------------------------------------------------------------------------------
    void scaleView( qreal scaleFactor ) noexcept;
    //没啥用的对外接口，一般用于方便连接信号或方便使用本类
    ICGraphicsScene *getScene() const noexcept;
    QGraphicsView *        getView() const noexcept;

    //---------------------------------------------------------------------------------
    /*!
     *@brief   重新设置Scene的尺寸，用于辅助设置图像
     */
    //----------------------------------------------------------------------------------
    void viewFitPixmapSize() noexcept;

protected:
    //---------------------------------------------------------------------------------
    /*!
     *@brief   滚轮事件：用于缩放视图
     */
    //----------------------------------------------------------------------------------
    virtual void wheelEvent( QWheelEvent *event ) noexcept override;

    Ui::ICPaintingView *ui;
    QGraphicsPixmapItem       pixmap;
    // QWidget interface

protected:
    bool isFilePathValid( const QFileInfo &info ) const noexcept;
};

#endif// ICPAINTINGVIEW_H
