#include "AbstractPropertyBrowserWindow.h"
#include "ui_AbstractPropertyBrowserWindow.h"

AbstractPropertyBrowserWindow::AbstractPropertyBrowserWindow(
        AbstractPropertyBrowser *browser , QWidget *parent ) noexcept
        : AbstractPropertyBrowserWindow( parent ) {
    assert( browser );
    connectToQtTreeBrowser( browser );
}

AbstractPropertyBrowserWindow::AbstractPropertyBrowserWindow(
        QWidget *parent ) noexcept :
        QMainWindow( parent ) , ui( new Ui::AbstractPropertyBrowserWindow ) {
    ui->setupUi( this );
    hide();
}

AbstractPropertyBrowserWindow::~AbstractPropertyBrowserWindow() {
    delete ui;
}

AbstractPropertyBrowser *AbstractPropertyBrowserWindow::getPropertyBrowser() noexcept {
    assert( browser );
    return browser;
}

void AbstractPropertyBrowserWindow::connectToQtTreeBrowser( AbstractPropertyBrowser *_browser ) noexcept {
    assert( _browser );
    this->browser = _browser;
    browser->setParent( this );
    ui->gridLayout->addWidget( browser , 0 , 0 , 1 , 1 );
    connect( ui->pushButton , &QPushButton::clicked , browser , &AbstractPropertyBrowser::setTableFromJson );
    connect( ui->pushButton_2 , &QPushButton::clicked , browser , &AbstractPropertyBrowser::setJsonFromTable );
}
