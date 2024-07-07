#ifndef ICWINDOW_H
#define ICWINDOW_H

#include "stable.h"
#include <QMainWindow>

#define FIRST_MODE TRAIN
class AbstractPropertyBrowser;

namespace Ui {
    class ICWindow;
}

class ICWindow: public QMainWindow {
    Q_OBJECT
    enum {
        TRAIN,
        EXCUTE
    };

public:
    explicit ICWindow( QWidget *parent = nullptr );
    ~ICWindow();
    //请使用该接口，返回 结果 与 结果彩色图 ，请输入灰色QImage对象
    static std::tuple< int, QImage > dealWithQImage( QImage gray_img ) noexcept;
    static std::tuple< int, QImage > dealWithMat( cv::Mat gray_img ) noexcept;
    // QWidget interface

protected:
    virtual void dragEnterEvent( QDragEnterEvent *event ) noexcept override;
    virtual void dropEvent( QDropEvent *event ) noexcept override;
    virtual void keyPressEvent( QKeyEvent *event ) noexcept override;
    void closeEvent(QCloseEvent *) noexcept override;

signals:
    void writeLog(QString, int);

    void sendMsg(QString);

private slots:
    void threshold1_changed_slot( int value ) noexcept;

    void on_comboBox_ModeSelect_currentIndexChanged( int index ) noexcept;

    void open_Images_action_slot() noexcept;

private:
    void                     dropEvent_Help( QStringList path_list ) noexcept;
    void                     execute() noexcept;
    static bool              isFilePathValid( const QString &info ) noexcept;
    static QFileInfoList     dealWithDirPath( const QFileInfo &info ) noexcept;
    Ui::ICWindow *    ui;
    int                      mode              = FIRST_MODE;
    AbstractPropertyBrowser *algorithm_browser = nullptr;
};

#endif// ICWINDOW_H
