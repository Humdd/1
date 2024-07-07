#include "AlgorithmPropertBrowserWindow.h"
#include "AlgorithmPropertBrowser.h"
#include "ui_AbstractPropertyBrowserWindow.h"

AlgorithmPropertBrowserWindow::AlgorithmPropertBrowserWindow( QWidget *parent ) noexcept
    : AbstractPropertyBrowserWindow( new AlgorithmPropertBrowser( "Config/Heat.json" ), parent ) {
    ui->pushButton->setVisible( false );
    ui->pushButton_2->setVisible( false );
}
