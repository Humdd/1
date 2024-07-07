#ifndef ABSTRACTPAINTINGVIEW_H
#define ABSTRACTPAINTINGVIEW_H
#include "stable.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class AbstractPaintingView;
}
QT_END_NAMESPACE

//---------------------------------------------------------------------------------
/*!
 *@brief   绘图的视图类（画布类），用于显示图片；“MVD”模式中的V（view），主要功能如下：
 *         （1）设置Scene、设置图片
 *         （2）放大、缩小视图
 *         （3）通过拖拽来改变显示的图片
 */
//----------------------------------------------------------------------------------
class AbstractPaintingView: public QWidget {
    Q_OBJECT

public:
    inline static std::vector< std::tuple< QString, std::unique_ptr< QGraphicsPixmapItem > > > img_info_array;
    inline static int                                                                          current_index;
    AbstractPaintingView( QWidget *parent = nullptr );
    ~AbstractPaintingView();
    //    static QVector<QString> &getImageArray() ;
    //---------------------------------------------------------------------------------
    /*!
     *@brief   初始化Scene
     */
    //----------------------------------------------------------------------------------
    void initScene( QGraphicsScene *scene );

    //---------------------------------------------------------------------------------
    /*!
     *@brief   用opencv的Mat来重新设置图片
     */
    //----------------------------------------------------------------------------------
    //    void setImage( const cv::Mat &_image );

    //---------------------------------------------------------------------------------
    /*!
     *@brief   用StepWindowInfo类的QPixmap（QT）来重新设置图片，为了要所有窗口显示图片一样
     */
    //----------------------------------------------------------------------------------
    void resetPixmap();
    //---------------------------------------------------------------------------------
    /*!
     *@brief   辅助缩放图像，这是真正缩放图像的函数
     */
    //----------------------------------------------------------------------------------
    void scaleView( qreal scaleFactor );
    void recordCurrentSceneRect();
    void makeSceneRectConsistent();
    void makeViewScaleConsistent();
    //没啥用的对外接口，一般用于方便连接信号或方便使用本类
    QGraphicsScene *    getScene() noexcept;
    QGraphicsView *     getGraphicsView();
    QGraphicsScene *    getGraphicsScene();
    void                clearPixmapArray();
    void                displayFirstPixmap();
    void                dropEventHelper( const QStringList &img_path_list );
    QGraphicsPixmapItem pixmap;
    //---------------------------------------------------------------------------------
    /*!
     *@brief   重新设置Scene的尺寸，用于辅助设置图像
     */
    //----------------------------------------------------------------------------------
    void viewFitPixmapSize();

protected:
    //---------------------------------------------------------------------------------
    /*!
     *@brief   滚轮事件：用于缩放视图
     */
    //----------------------------------------------------------------------------------
    virtual void wheelEvent( QWheelEvent *event ) override;

    //---------------------------------------------------------------------------------
    /*!
     *@brief   拖拽事件：用于拖拽图片来重新设置视图显示的图像
     */
    //----------------------------------------------------------------------------------
    virtual void dragEnterEvent( QDragEnterEvent *event ) override;
    //---------------------------------------------------------------------------------
    /*!
     *@brief   拖拽事件：用于拖拽图片来重新设置视图显示的图像
     */
    //----------------------------------------------------------------------------------
    virtual void              dropEvent( QDropEvent *event ) override;
    Ui::AbstractPaintingView *ui;

    // QWidget interface

protected:
    virtual void  keyPressEvent( QKeyEvent *event ) override;
    bool          isFilePathValid( const QFileInfo &info ) const;
    QFileInfoList dealWithDirPath( const QFileInfo &info ) const;
};

#endif// ABSTRACTPAINTINGVIEW_H
