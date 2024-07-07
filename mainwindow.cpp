#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <windows.h>
#define DEPLOY

MainWindow::MainWindow( QWidget *parent )
    : QMainWindow( parent )
    , ui( new Ui::MainWindow )
    , m_pDeviceProcessCurrent(nullptr)
    , m_nDeviceListCurrent(0)
    , tcp_server_(new TcpServer(this))
    , current_socket(nullptr)
    , socket_ic(nullptr)
    , socket_heat(nullptr)
    , cnt_client(0)
    , m_bIsConnected(false)
    , m_bIsCmdFromLocal(false)
    , m_bIsInTestMode(false)
    , m_bCheckShowDevice( false ) {
    // 加载 UI 文件，并将控件和布局信息关联到成员变量中
    ui->setupUi( this );
    //初始化状态栏
    m_pLLabelStaBar = new QLabel( this );
    m_pRLabelStaBar = new QLabel( this );
    statusBar()->addWidget( m_pLLabelStaBar );
    statusBar()->addWidget( m_pRLabelStaBar );
    //获取显示区域子窗口句柄
    pLBitmap = (CGXBitmap *)ui->bitMap_LCam;
    pRBitmap = (CGXBitmap *)ui->bitMap_RCam;
    //初始化设备处理类
    for ( int i = 0; i < DEVICE_COUNTS; i++ ) {
        m_pDeviceProcess[i] = nullptr;
    }

    //初始化API
    __InitCGXAPI();

    //初始化控制指令
    ui->comboBox_Command->addItem("WakeUp ic");
    ui->comboBox_Command->addItem("WakeUp heat");
    ui->comboBox_Command->addItem("TakePhoto ic");
    ui->comboBox_Command->addItem("TakePhoto heat");
    ui->comboBox_Command->addItem("Test ic");
    ui->comboBox_Command->addItem("Test heat");

    //日志窗口
    ui->textEdit_LogWindow->document()->setMaximumBlockCount(200); //设置最大行数
    textCursor = ui->textEdit_LogWindow->textCursor();//返回文本编辑器中的当前光标位置和所选文本的范围。

    //连接信号和槽函数
    //相机选择
    connect(ui->cB_DeviceList, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &MainWindow::on_Cb_DeviceList_currentIndexChanged );


    connect(m_pDeviceProcess[0], &CDeviceProcess::send_to_mainwind, this, &MainWindow::sendMsg);
    connect(m_pDeviceProcess[1], &CDeviceProcess::send_to_mainwind, this, &MainWindow::sendMsg);

    //没用到
    connect(m_pDeviceProcess[0], &CDeviceProcess::writeLog, this, &MainWindow::writeLog);
    connect(m_pDeviceProcess[1], &CDeviceProcess::writeLog, this, &MainWindow::writeLog);

    connect(&iCWindow, &ICWindow::writeLog, this, &MainWindow::writeLog);
    connect(&iCWindow, &ICWindow::sendMsg, this, &MainWindow::sendMsg);
    connect(&filmStripWindow, &FilmStripWindow::writeLog, this, &MainWindow::writeLog);
    connect(&filmStripWindow, &FilmStripWindow::sendMsg, this, &MainWindow::sendMsg);

    //tcp
    connect(this, &MainWindow::ServerRecved, this, &MainWindow::ServerRecvedSlot);
    connect(tcp_server_, &TcpServer::ClientConnected, this, &MainWindow::ClientConnected); //信号槽
    connect(tcp_server_, &TcpServer::ClientDisconnected, this, &MainWindow::ClientDisconnected); //监听
}

MainWindow::~MainWindow() {
    tcp_server_->close();
    delete ui;
}


void MainWindow::closeEvent( QCloseEvent *event ) {
    //在函数执行开始时记录函数执行时间
    NoteTimingStart

    try {
        //遍历map容器，将已打开设备全部关闭
        map<int, CDeviceProcess *>::iterator it;

        for ( it = m_mapDeviceInformation.begin(); it != m_mapDeviceInformation.end(); ++it ) {
            it->second->CloseDevice();
        }

        m_mapDeviceInformation.clear();
    }
    catch ( CGalaxyException ) {
        //do noting
    }
    catch ( std::exception ) {
        //do noting
    }

    try {
        //释放设备库资源
        IGXFactory::GetInstance().Uninit();
    }
    catch ( CGalaxyException ) {
        //do noting
    }

    for ( int i = 0; i < DEVICE_COUNTS; i++ ) {
        if ( m_pDeviceProcess[i] != nullptr ) {
            delete m_pDeviceProcess[i];
            m_pDeviceProcess[i] = nullptr;
        }
    }

    event->accept();
}


void MainWindow::__InitCGXAPI() {
    try {
        for ( int i = 0; i < DEVICE_COUNTS; i++ ) {
            m_pDeviceProcess[i] = new CDeviceProcess;
        }
        //绑定显示控件
        m_pDeviceProcess[0]->setBitmap(pRBitmap);
        m_pDeviceProcess[1]->setBitmap(pLBitmap);

        //初始化设备库
        IGXFactory::GetInstance().Init();
        //枚举设备
        IGXFactory::GetInstance().UpdateDeviceList( 1000, m_vectorDeviceInfo );

        //未检测到设备
        if ( m_vectorDeviceInfo.size() <= 0 ) {
            m_pRLabelStaBar->setText( "未检测到设备,请确保设备正常连接!" );
            return;
        }

        //将设备名称显示到设备列表中
        for ( uint32_t i = 0; i < m_vectorDeviceInfo.size(); i++ ) {
            //如果设备数大于2则只保留2个设备信息
            if ( i >= DEVICE_COUNTS ) {
                break;
            }
            gxstring strDeviceInformation = "";
//            strDeviceInformation = m_vectorDeviceInfo[i].GetDisplayName();
            strDeviceInformation = m_vectorDeviceInfo[i].GetSN();
            ui->cB_DeviceList->addItem( strDeviceInformation.c_str() );
            m_mapDeviceInformation.insert( map<int, CDeviceProcess *>::value_type( i, m_pDeviceProcess[i] ) );
            //更新界面
            __UpdateUI( m_pDeviceProcess[i] );
        }

        ui->cB_DeviceList->setCurrentIndex( 0 );
        //获取当前显示的设备处理类对象
        m_pDeviceProcessCurrent = m_mapDeviceInformation[0];
    }
    catch ( CGalaxyException &e ) {
        m_pRLabelStaBar->setText( e.what() );
        return;
    }
    catch ( std::exception &e ) {
        m_pRLabelStaBar->setText( e.what() );
        return;
    }
}

bool MainWindow::checkDevice()
{
    if (m_vectorDeviceInfo.size() != 2)
    {
        QMessageBox::critical(nullptr, "警告", "请确保打开两台相机");
        Q_LOG4CPLUS_WARN("Please make sure there are two devices.");
        return false;
    }
    if (!m_pDeviceProcess[0]->IsOpen() && !m_pDeviceProcess[0]->IsSnap())
    {
        QMessageBox::critical(nullptr, "警告", "请确保打开ic相机");
        Q_LOG4CPLUS_WARN("Please open ic device");
        return false;
    }
    if (!m_pDeviceProcess[1]->IsOpen() && !m_pDeviceProcess[1]->IsSnap())
    {
        QMessageBox::critical(nullptr, "警告", "请确保打开热合相机");
        Q_LOG4CPLUS_WARN("Please open heat device");
        return false;
    }
    return true;
}



void MainWindow::processRecvMsg(QString recvMsg)
{
    NoteTimingStart;
    //拍照
    if (recvMsg.startsWith("TakePhoto"))
    {
        if (!m_bIsInTestMode)
        {
            QMessageBox::warning(this, "提示", "请打开检测模式");
            Q_LOG4CPLUS_WARN("Please open Test mode");
            return;
        }
        if (!checkDevice())
            return;

        if (recvMsg == "TakePhoto ic")
        {
            m_pDeviceProcess[0]->setSaveICImageFromSocket(true);
            m_pDeviceProcess[0]->triggerSoftware();
        }
        else if (recvMsg == "TakePhoto heat")
        {
            m_pDeviceProcess[1]->setSaveHeatImageFromSocket(true);
            m_pDeviceProcess[1]->triggerSoftware();
        }
    }

    //标注
    if (recvMsg == "WakeUp ic")
    {
        openICAnnotation();
    }
    else if (recvMsg == "WakeUp heat")
    {
        openHeatAnnotation();
    }

    //检测
    if (recvMsg.startsWith("Test"))
    {
        if (!m_bIsInTestMode)
        {
            QMessageBox::warning(this, "提示", "请打开检测模式");
            Q_LOG4CPLUS_WARN("Please open Test mode");
            return;
        }
        if (!checkDevice())
            return;

        // ic检测
        if (recvMsg == "Test ic")
        {
            m_pDeviceProcess[0]->setDetectICFromSocket(true);
            m_pDeviceProcess[0]->triggerSoftware();
        }
        // 热合检测
        else if (recvMsg == "Test heat")
        {
            m_pDeviceProcess[1]->setDetectHeatFromSocket(true);
            m_pDeviceProcess[1]->triggerSoftware();
        }
    }

    // 本地测试专用
    if (recvMsg.startsWith("test ic"))
    {
        //test ic ./images/cam1-20230317-094531-843.bmp
        Q_LOG4CPLUS_INFO("start: test ic")

        auto stringList = recvMsg.split(" ");
        if (stringList.size() < 3)
        {
            writeLog("命令有误，请检查图片路径是否正确!", 1);
            return;
        }
        QString filePath = stringList[2];
        filePath = filePath.trimmed(); //移前后空格
        filePath.replace("\"", ""); //移除双引号
        filePath = QDir::fromNativeSeparators(filePath); //适配反斜杠路径
        //判断文件是否存在
        if (!QFile::exists(filePath))
        {
            std::cerr << "not exist" << std::endl;
            writeLog("文件不存在，请检查图片路径是否正确!", 1);
            return;
        }

        QString baseName = QFileInfo(filePath).baseName();
        cv::Mat mat = cv::imread(filePath.toStdString(), cv::IMREAD_GRAYSCALE);
        QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8); //image.format()底层数字是24

        if (mat.empty())
        {
            std::cerr << "mat.empty()" << std::endl;
            return;
        }
        if (image.isNull())
        {
            std::cerr << "image.isNull()" << std::endl;
            return;
        }

        Q_LOG4CPLUS_INFO("end: load image")
        const auto &[result_code, image_result] = ICWindow::dealWithQImage(image); //结构化绑定，获取tuple
        Q_LOG4CPLUS_INFO("end: ICWindow::dealWithQImage(image)")
        std::cout << result_code << std::endl;
        QString msg = QString("test ic %1").arg(QString::number(result_code));
        sendMsg(msg);

        if (image_result.isNull())
        {
            std::cerr << "image_result.isNull()" << std::endl;
            return;
        }
        QString strDeviceSN = "";
        m_pDeviceProcess[0]->showImage(image_result, strDeviceSN);
        //保存原图和结果图
        Q_LOG4CPLUS_INFO("end: showImage")
        QString saveDir = "./images/ic/";
        if (!QDir().exists(saveDir))
        {
            QDir().mkpath(saveDir);
        }
        QString savePath_src = saveDir + baseName + ".bmp";
        QString savePath_result = saveDir + baseName + "-result.bmp";
        image.save(savePath_src);
        Q_LOG4CPLUS_INFO("end: save image_src")
        image_result.save(savePath_result);
        Q_LOG4CPLUS_INFO("end: save image_result")
        Q_LOG4CPLUS_INFO("end: test ic")
    }
    else if (recvMsg.startsWith("test heat"))
    {
        Q_LOG4CPLUS_INFO("start: test heat")
        auto stringList = recvMsg.split(" ");
        if (stringList.size() < 3)
        {
            writeLog("命令有误，请检查图片路径是否正确!", 1);
            return;
        }
        QString filePath = stringList[2];
        filePath = filePath.trimmed(); //移前后空格
        filePath.replace("\"", ""); //移除双引号
        filePath = QDir::fromNativeSeparators(filePath); //适配反斜杠路径
        //判断文件是否存在
        if (!QFile::exists(filePath))
        {
            std::cerr << "not exist" << std::endl;
            writeLog("文件不存在，请检查图片路径是否正确!", 1);
            return;
        }
        QString baseName = QFileInfo(filePath).baseName();
        cv::Mat mat = cv::imread(filePath.toStdString(), cv::IMREAD_GRAYSCALE);
        QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8); //image.format()底层数字是24

        if (mat.empty())
        {
            std::cerr << "mat.empty()" << std::endl;
            return;
        }
        if (image.isNull())
        {
            std::cerr << "image.isNull()" << std::endl;
            return;
        }

        Q_LOG4CPLUS_INFO("end: load image")
        const auto &[result_code, image_result] = FilmStripWindow::dealWithQImage(image); //结构化绑定，获取tuple
        Q_LOG4CPLUS_INFO("end: FilmStripWindow::dealWithQImage(image)")
        std::cout << result_code << std::endl;
        QString msg = QString("test heat %1").arg(QString::number(result_code));
        sendMsg(msg);

        if (image_result.isNull())
        {
            std::cerr << "image_result.isNull()" << std::endl;
            return;
        }
        QString strDeviceSN = "";
        m_pDeviceProcess[1]->showImage(image_result, strDeviceSN);
        Q_LOG4CPLUS_INFO("end: showImage")
        //保存原图和结果图
        QString saveDir = "./images/heat/";
        if (!QDir().exists(saveDir))
        {
            QDir().mkpath(saveDir);
        }
        QString savePath_src = saveDir + baseName + ".bmp";
        QString savePath_result = saveDir + baseName + "-result.bmp";
        image.save(savePath_src);
        Q_LOG4CPLUS_INFO("end: save image_src")
        image_result.save(savePath_result);
        Q_LOG4CPLUS_INFO("end: save image_result")
        Q_LOG4CPLUS_INFO("end: test heat")
    }
}

void MainWindow::writeLog(const QString &str, const int mode)
{
    //将一个给定的字符串 str 与当前日期和时间信息拼接起来，并将结果存储在一个新的字符串变量 dst_str 中。这通常用于在日志中记录时间戳
    const auto &dst_str = QDateTime::currentDateTime().toString ("yyyy-MM-dd_hh-mm-ss") + " : " + str + "\n";
    if (mode == 0)
    {
        format.setForeground(QBrush(Qt::black));
        textCursor.setCharFormat(format);
        textCursor.insertText(dst_str);
        ui->textEdit_LogWindow->moveCursor(QTextCursor::End);
    }
    else if (mode == 1)
    {
        format.setForeground(QBrush(Qt::red));
        textCursor.setCharFormat(format);
        textCursor.insertText(dst_str);
        ui->textEdit_LogWindow->moveCursor(QTextCursor::End);
    }
}

void MainWindow::openICAnnotation()
{
    iCWindow.showMaximized();
    iCWindow.show();
}

void MainWindow::openHeatAnnotation()
{
    filmStripWindow.showMaximized();
    filmStripWindow.show();
}

void MainWindow::ClientConnected(qintptr handle, QTcpSocket *socket)
{
    m_bIsConnected = true;
    cnt_client++;
    quint32 addr_origin = socket->peerAddress().toIPv4Address();
    QHostAddress addr_host = QHostAddress(addr_origin);
    QString addr_str = addr_host.toString();
    QString addr_port_str = addr_str + ':' + QString::number(socket->peerPort());
    QString logContent = QString("connect %1").arg(addr_port_str);
    writeLog(logContent, 0);
    Q_LOG4CPLUS_INFO(logContent)
    connect(socket, &QTcpSocket::readyRead,
            [=]()
            {
                emit ServerRecved(handle, socket, socket->readAll());
            });
}

void MainWindow::ClientDisconnected(qintptr handle)
{
    cnt_client--;
    if (cnt_client == 0)
        m_bIsConnected = false;

    qDebug() << QString("qintptr handle %1 disconnected").arg(QString::number(handle));
}

void MainWindow::ServerRecvedSlot(qintptr handle, QTcpSocket *socket, const QByteArray &data)//qintptr是一个平台相关的整数类型
{
    Q_UNUSED(handle);//Q_UNUSED()是一个宏，用于告诉编译器某个变量未被使用，以避免编译器产生“未使用变量”的警告。

    //获取地址端口信息，如127.0.0.1:8080
    quint32 addr_origin = socket->peerAddress().toIPv4Address();
    QHostAddress addr_host = QHostAddress(addr_origin);
    QString addr_str = addr_host.toString();
    QString addr_port_str = addr_str + ':' + QString::number(socket->peerPort());
    QString recvMsg(data);
    QString logContent = QString("Receive from %1: %2").arg(addr_port_str, recvMsg);

    writeLog(logContent, 0);
    Q_LOG4CPLUS_INFO(logContent)

    if (recvMsg.startsWith("WakeUp ic") || recvMsg.startsWith("TakePhoto ic") || recvMsg.startsWith("Test ic"))
    {
        socket_ic = socket;
    }
    else if (recvMsg.startsWith("WakeUp heat") || recvMsg.startsWith("TakePhoto heat") || recvMsg.startsWith("Test heat"))
    {
        socket_heat = socket;
    }
    else
    {
        current_socket = socket;
    }

    processRecvMsg(recvMsg);
}

void MainWindow::testShowImage()
{
    qDebug() << "test show";
    NoteTimingStart;
    //获取显示区域子窗口句柄
    CGXBitmap *pRBitmap = (CGXBitmap *)ui->bitMap_LCam;
    QImage image = QImage("./images/20220802-cam1-dataset/0000_8.bmp");
    if (image.isNull())
    {
        qDebug() << "image is null";
        return;
    }
    QString strDeviceSN("FCW22040086");
    pRBitmap->showImage(image, strDeviceSN);
    cv::Mat mat(image.height(), image.width(),
                CV_8UC1,
                const_cast<uchar*>(image.bits()),
                static_cast<size_t>(image.bytesPerLine())
                );
    cv::Rect roi(1595, 820, 830, 1245);
    cv::Mat chip_roi(mat(roi));
    int nType = m_pDeviceProcess[0]->getType(chip_roi);
    qDebug() << "[nType]: " << nType;
}

void MainWindow::__InitUI( CGXFeatureControlPointer objFeatureControlPtr ) {
    if ( objFeatureControlPtr.IsNull() ) {
        return;
    }

    //将当前设备是否显示设备型号刷到界面
    m_bCheckShowDevice = m_pDeviceProcessCurrent->GetShowSN();
    ui->ckB_ShowDeviceSN->setChecked( m_bCheckShowDevice );
}

void MainWindow::__UpdateUI( CDeviceProcess *pDeviceProcess ) {
    if ( pDeviceProcess == nullptr ) {
        return;
    }

    //    bool IsOpen = pDeviceProcess->IsOpen();
    //    bool IsSnap = pDeviceProcess->IsSnap();
    bool IsOpen = ( m_pDeviceProcess[0]->IsOpen() && m_pDeviceProcess[0]->IsSnap() && m_pDeviceProcess[1]->IsOpen() && m_pDeviceProcess[1]->IsSnap() );
    //    bool IsOpenAndSnap = pDeviceProcess->IsOpenAndSnap();
    //    ui->pB_OpenDevice->setEnabled( !IsOpen );
    //    ui->pB_CloseDevice->setEnabled( IsOpen );
    //    ui->pB_StartCapture->setEnabled( IsOpen && !IsSnap );
    //    ui->pB_StopCapture->setEnabled( IsOpen && IsSnap );
    //    ui->pB_Open->setEnabled(!IsOpenAndSnap);
    //    ui->pB_Close->setEnabled(IsOpenAndSnap);
    ui->ckB_ShowDeviceSN->setEnabled( IsOpen );
}

void MainWindow::on_Cb_DeviceList_currentIndexChanged( int nIndex ) {

    try {
        std::swap(m_pDeviceProcess[0], m_pDeviceProcess[1]);
        std::swap(m_mapDeviceInformation[0], m_mapDeviceInformation[1]);
        m_pDeviceProcess[0]->setBitmap(pRBitmap);
        m_pDeviceProcess[1]->setBitmap(pLBitmap);
        m_pDeviceProcess[0]->setDeviceIndex(0);
        m_pDeviceProcess[1]->setDeviceIndex(1);

//        m_nDeviceListCurrent = nIndex;
//        m_pDeviceProcessCurrent = m_mapDeviceInformation[m_nDeviceListCurrent];
//        //判断当前设备是否已打开
//        bool bIsOpen = m_pDeviceProcessCurrent->IsOpen();

//        if ( !bIsOpen ) {
//            __UpdateUI( m_pDeviceProcessCurrent );
//        }
//        else {
//            //刷新界面
//            __InitUI( m_pDeviceProcessCurrent->m_objFeatureControlPtr );
//            //更新界面
//            __UpdateUI( m_pDeviceProcessCurrent );
//        }
    }
    catch ( CGalaxyException &e ) {
        m_pRLabelStaBar->setText( e.what() );
        return;
    }
    catch ( std::exception &e ) {
        m_pRLabelStaBar->setText( e.what() );
        return;
    }
}

void MainWindow::on_pB_StartServer_clicked() {
#ifdef DEPLOY
    if (m_vectorDeviceInfo.size() != 2)
    {
        Q_LOG4CPLUS_WARN("No device.");
        QMessageBox::warning(nullptr, "提示信息", "请连接相机");
        return;
    }
#endif

    //监听端口
    uint16_t port = ui->lE_Port->text().toUShort();
    tcp_server_->listen(QHostAddress::Any, port); //9090
    writeLog("start server", 0);
    Q_LOG4CPLUS_INFO( "start server" );
    ui->pB_StartServer->setEnabled( false );
    ui->pB_StopServer->setEnabled( true );
    ui->lE_Port->setEnabled( false );
//    ui->lE_ShowFrameInterval->setEnabled( false );
//    ui->lE_SaveFrameInterval->setEnabled( false );
    ui->lE_SaveFileNameListSize->setEnabled( false );
    ui->ckB_TestMode->setEnabled( true );
    ui->pB_refreshDeviceList->setEnabled(false);

    // 打开设备并开始采集
#ifdef DEPLOY
    try {
        //打开设备，默认第一台设备显示在右边
        m_pDeviceProcess[0]->OpenDevice( m_vectorDeviceInfo[0].GetSN(), pRBitmap, 0 );
        m_pDeviceProcess[1]->OpenDevice( m_vectorDeviceInfo[1].GetSN(), pLBitmap, 1 );
        //设置保存图片列表大小
        m_pDeviceProcess[0]->setSaveFileNameListSize( ui->lE_SaveFileNameListSize->text().toInt() );
        m_pDeviceProcess[1]->setSaveFileNameListSize( ui->lE_SaveFileNameListSize->text().toInt() );
        //设置保存图片帧间隔、显示帧间隔、否需要显示图像、保存图像
        m_pDeviceProcess[0]->setSaveFrameInterval( ui->lE_SaveFrameInterval->text().toInt() );
        m_pDeviceProcess[1]->setSaveFrameInterval( ui->lE_SaveFrameInterval->text().toInt() );
        m_pDeviceProcess[0]->setSaveImage( ui->ckB_SaveImage->isChecked() );
        m_pDeviceProcess[1]->setSaveImage( ui->ckB_SaveImage->isChecked() );
        m_pDeviceProcess[0]->setShowFrameInterval( ui->lE_ShowFrameInterval->text().toInt() );
        m_pDeviceProcess[1]->setShowFrameInterval( ui->lE_ShowFrameInterval->text().toInt() );
//        m_pDeviceProcess[0]->setShowImage( ui->ckB_ShowImage->isChecked() );
//        m_pDeviceProcess[1]->setShowImage( ui->ckB_ShowImage->isChecked() );
        //更新设备型号及帧率信息
        m_pDeviceProcess[0]->RefreshDeviceSN();
        m_pDeviceProcess[1]->RefreshDeviceSN();
        //开始采集
        m_pDeviceProcess[0]->StartSnap();
        m_pDeviceProcess[1]->StartSnap();
        //初始化界面
        __InitUI( m_pDeviceProcessCurrent->m_objFeatureControlPtr );
        //更新界面
        __UpdateUI( m_pDeviceProcessCurrent );

//        //开启视频
//        if (!m_bIsInTestMode)
//        {
//            Q_LOG4CPLUS_INFO("close TestMode")
//            m_pDeviceProcess[0]->showRecursivelyNotInTestMode();
//            m_pDeviceProcess[1]->showRecursivelyNotInTestMode();
//        }
    }
    catch ( CGalaxyException &e ) {
        m_pRLabelStaBar->setText( e.what() );
        return;
    }
    catch ( std::exception &e ) {
        m_pRLabelStaBar->setText( e.what() );
        return;
    }
#endif
}

void MainWindow::on_pB_StopServer_clicked() {
    if ( m_bIsConnected ) {
        current_socket->abort();
        m_bIsConnected = false;
    }

    if ( tcp_server_->isListening() ) {
        tcp_server_->close();
    }

    ui->pB_StartServer->setEnabled( true );
    ui->pB_StopServer->setEnabled( false );
    ui->lE_Port->setEnabled( true );
//    ui->lE_ShowFrameInterval->setEnabled( true );
//    ui->lE_SaveFrameInterval->setEnabled( true );
    ui->lE_SaveFileNameListSize->setEnabled( true );
//    ui->ckB_ShowImage->setEnabled( false );
    ui->pB_refreshDeviceList->setEnabled(true);
    writeLog("stop server", 0);
    Q_LOG4CPLUS_INFO("stop server");

    // 关闭服务器，则停止采集并关闭设备
    try {
        //停止采集
        m_pDeviceProcess[0]->StopSnap();
        m_pDeviceProcess[1]->StopSnap();
        //关闭设备
        m_pDeviceProcess[0]->CloseDevice();
        m_pDeviceProcess[1]->CloseDevice();
        //更新界面
        __UpdateUI( m_pDeviceProcessCurrent );
    }
    catch ( CGalaxyException ) {
        // do noting
    }
    catch ( std::exception ) {
        // do noting
        return;
    }
}

void MainWindow::on_ckB_SaveImage_toggled(bool checked)
{
    ui->lE_SaveFrameInterval->setEnabled(!checked);
    m_pDeviceProcess[0]->setSaveFrameInterval(ui->lE_SaveFrameInterval->text().toInt());
    m_pDeviceProcess[1]->setSaveFrameInterval(ui->lE_SaveFrameInterval->text().toInt());
    m_pDeviceProcess[0]->setSaveImage(checked);
    m_pDeviceProcess[1]->setSaveImage(checked);
}

void MainWindow::on_ckB_ShowImage_toggled(bool checked)
{
    ui->lE_ShowFrameInterval->setEnabled(!checked);
    m_pDeviceProcess[0]->setShowFrameInterval( ui->lE_ShowFrameInterval->text().toInt() );
    m_pDeviceProcess[1]->setShowFrameInterval( ui->lE_ShowFrameInterval->text().toInt() );
    m_pDeviceProcess[0]->setShowImage(checked);
    m_pDeviceProcess[1]->setShowImage(checked);
}

void MainWindow::on_ckB_TestMode_toggled(bool checked)
{
    m_bIsInTestMode = checked;
    m_pDeviceProcess[0]->setTestMode(m_bIsInTestMode);
    m_pDeviceProcess[1]->setTestMode(m_bIsInTestMode);
    if (m_bIsInTestMode)
    {
        m_pDeviceProcess[0]->setTriggerMode("On"); //“On”主动调用回调函数，“Off”自动调用回调函数
        m_pDeviceProcess[1]->setTriggerMode("On");
    }
    else
    {
        m_pDeviceProcess[0]->setTriggerMode("Off");
        m_pDeviceProcess[1]->setTriggerMode("Off");
    }
//    //开启视频流
//    if (!m_bIsInTestMode)
//    {
//        Q_LOG4CPLUS_INFO("close TestMode")
//        m_pDeviceProcess[0]->showRecursivelyNotInTestMode();
//        m_pDeviceProcess[1]->showRecursivelyNotInTestMode();
//    }
//    else
//        Q_LOG4CPLUS_INFO("open TestMode")
}

void MainWindow::on_ckB_ShowDeviceSN_toggled(bool checked)
{
    m_pDeviceProcess[0]->SetShowSN( checked );
    m_pDeviceProcess[1]->SetShowSN( checked );
}

void MainWindow::on_pB_refreshDeviceList_clicked()
{
    try {
        //遍历map容器，将已打开设备全部关闭
        map<int, CDeviceProcess *>::iterator it;

        for ( it = m_mapDeviceInformation.begin(); it != m_mapDeviceInformation.end(); ++it ) {
            it->second->CloseDevice();
        }

        //清空设备信息容器
        m_mapDeviceInformation.clear();
        m_vectorDeviceInfo.clear();
    }
    catch ( CGalaxyException ) {
        //do noting
    }
    catch ( std::exception ) {
        //do noting
    }

    //删除所有对象
    for ( int i = 0; i < DEVICE_COUNTS; i++ ) {
        if ( m_pDeviceProcess[i] != nullptr ) {
            delete m_pDeviceProcess[i];
            m_pDeviceProcess[i] = nullptr;
        }
    }

//    disconnect( ui->cB_DeviceList, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &MainWindow::on_cB_DeviceList_currentIndexChanged );
    disconnect( ui->cB_DeviceList, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &MainWindow::on_Cb_DeviceList_currentIndexChanged );

    try {
        for ( int i = 0; i < DEVICE_COUNTS; i++ ) {
            m_pDeviceProcess[i] = new CDeviceProcess;
        }

        //枚举设备
        IGXFactory::GetInstance().UpdateDeviceList( 1000, m_vectorDeviceInfo );

        //未检测到设备
        if ( m_vectorDeviceInfo.size() <= 0 ) {
            m_pRLabelStaBar->setText( "未检测到设备,请确保设备正常连接!" );
            return;
        }

        ui->cB_DeviceList->clear();

        //将设备名称显示到设备列表中
        for ( uint32_t i = 0; i < m_vectorDeviceInfo.size(); i++ ) {
            //如果设备数大于2则只保留2个设备信息
            if ( i >= DEVICE_COUNTS ) {
                break;
            }

            gxstring strDeviceInformation = "";
            strDeviceInformation = m_vectorDeviceInfo[i].GetDisplayName();
            ui->cB_DeviceList->addItem( strDeviceInformation.c_str() );
            m_mapDeviceInformation.insert( map<int, CDeviceProcess *>::value_type( i, m_pDeviceProcess[i] ) );
            //更新界面
            __UpdateUI( m_pDeviceProcess[i] );
        }

        ui->cB_DeviceList->setCurrentIndex( 0 );
//        connect( ui->cB_DeviceList, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &MainWindow::on_cB_DeviceList_currentIndexChanged );
        connect( ui->cB_DeviceList, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &MainWindow::on_Cb_DeviceList_currentIndexChanged );
        //获取当前显示的设备处理类对象
        m_pDeviceProcessCurrent = m_mapDeviceInformation[0];
    }
    catch ( CGalaxyException &e ) {
//        connect( ui->cB_DeviceList, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &MainWindow::on_cB_DeviceList_currentIndexChanged );
        connect( ui->cB_DeviceList, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &MainWindow::on_Cb_DeviceList_currentIndexChanged );
        m_pRLabelStaBar->setText( e.what() );
        return;
    }
    catch ( std::exception &e ) {
//        connect( ui->cB_DeviceList, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &MainWindow::on_cB_DeviceList_currentIndexChanged );
        connect( ui->cB_DeviceList, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &MainWindow::on_Cb_DeviceList_currentIndexChanged );
        m_pRLabelStaBar->setText( e.what() );
        return;
    }
}

void MainWindow::sendMsg(QString sendMsg)//发送指令
{
    if (m_bIsCmdFromLocal)//m_bIsCmdFromLocal 命令是否本地触发
    {
        QString logContent = QString("Local result: %1").arg(sendMsg);//"%1" 替换为变量sendMsg的值
        writeLog(logContent, 0);
        Q_LOG4CPLUS_INFO(logContent)//日志记录器
        qDebug() << logContent;
        m_bIsCmdFromLocal = false;
    }
    else if (m_bIsConnected)//m_bIsConnected 是否建立Tcp连接标志
    {
        if (sendMsg.startsWith("WakeUp ic") || sendMsg.startsWith("TakePhoto ic") || sendMsg.startsWith("Test ic"))
        {
            if (socket_ic == nullptr)
            {
                Q_LOG4CPLUS_ERROR("socket_ic == nullptr")
                qDebug() << "socket_ic == nullptr";
                return;
            }
            current_socket = socket_ic;
        }
        else if (sendMsg.startsWith("WakeUp heat") || sendMsg.startsWith("TakePhoto heat") || sendMsg.startsWith("Test heat"))
        {
            if (socket_heat == nullptr)
            {
                Q_LOG4CPLUS_ERROR("socket_heat == nullptr")
                qDebug() << "socket_heat == nullptr";
                return;
            }
            current_socket = socket_heat;
        }

        if (current_socket == nullptr)
        {
            Q_LOG4CPLUS_ERROR("current_socket == nullptr")
            qDebug() << "current_socket == nullptr";
            return;
        }
        //获取地址端口信息，如127.0.0.1:8080
        quint32 addr_origin = current_socket->peerAddress().toIPv4Address();//获取远程主机的IPv4地址，并将其转换为32位无符号整数。
        QHostAddress addr_host = QHostAddress(addr_origin);//QHostAddress是一个用于表示IP地址的类
        QString addr_str = addr_host.toString();
        QString addr_port_str = addr_str + ':' + QString::number(current_socket->peerPort());

        QString logContent = QString("Send Msg to %1: %2").arg(addr_port_str, sendMsg);
        writeLog(logContent, 0);
        Q_LOG4CPLUS_INFO(logContent)
        qDebug() << logContent;
        current_socket->write(sendMsg.toUtf8());//发送了一个HTTP请求
    }
}

//按下发送指令后跳转到processRecvMsg(cmd)函数
void MainWindow::on_pushButton_SendCommand_clicked()
{
    m_bIsCmdFromLocal = true;
    QString cmd = ui->comboBox_Command->currentText();
    QString logContent = QString("Receive local command: %1").arg(cmd);
    writeLog(logContent, 0);
    Q_LOG4CPLUS_INFO(logContent)
    processRecvMsg(cmd);
}




