#include "ICAlgorithmPropertBrowserWindow.h"
#include "ICAlgorithmPropertBrowser.h"
#include "ui_AbstractPropertyBrowserWindow.h"

ICAlgorithmPropertBrowserWindow::ICAlgorithmPropertBrowserWindow( QWidget *parent ) noexcept
    : AbstractPropertyBrowserWindow( new ICAlgorithmPropertBrowser( "Config/IC.json" ), parent ) {
    ui->pushButton->setVisible( false );
    ui->pushButton_2->setVisible( false );
}
