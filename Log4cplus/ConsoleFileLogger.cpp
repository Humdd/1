#include "Log4cplus/ConsoleFileLogger.h"

ConsoleFileLogger::ConsoleFileLogger( QObject *parent )
    : QObject( parent ) {
}

ConsoleFileLogger *ConsoleFileLogger::getInstance() {
    static ConsoleFileLogger *dst = new ConsoleFileLogger;
    return dst;
}

void ConsoleFileLogger::close() {}

std::wstring ConsoleFileLogger::logContent( const QString &str ) {
    return str.toStdWString();
}

void ConsoleFileLogger::timingStart() {
    m_timer.timingStart();
}

void ConsoleFileLogger::lastingTime( const QString &str ) {
    Q_LOG4CPLUS_INFO( m_timer.outputContent( str ) );
}

ConsoleFileLogger::~ConsoleFileLogger() {
    destructorImpl();
}

log4cplus::Logger &GetLogger::getLogger() {
    static log4cplus::Initializer       initializer;
    static log4cplus::SharedAppenderPtr consoleAppender( new log4cplus::ConsoleAppender( false, true ) );
    //    static log4cplus::SharedAppenderPtr qtAppender( ConsoleFileLogger::getInstance() );
    static log4cplus::SharedAppenderPtr fileAppender(
        new log4cplus::RollingFileAppender(
            /*   set the log path in here! */
            LOG4CPLUS_TEXT( "./log/detection.log" ),
            1024 * 1024 * 200, 10 ) );
    static log4cplus::Logger logger = log4cplus::Logger::getInstance( LOG4CPLUS_TEXT( "logger" ) );
    static const bool        temp   = [ & ]() {
        onlyInDebug(
            log4cplus::tstring pattern = LOG4CPLUS_TEXT( "%D{%m/%d/%y %H:%M:%S,%Q} [%t] %-5p: %m [file://%l]%n" ); );
        onlyInRelease(
            log4cplus::tstring pattern = LOG4CPLUS_TEXT( "%D{%m/%d/%y %H:%M:%S,%Q} [%t] %-5p: %m%n" ); );
        consoleAppender->setName( LOG4CPLUS_TEXT( "console" ) );
        consoleAppender->setLayout( std::unique_ptr< log4cplus::Layout >( new log4cplus::PatternLayout( pattern ) ) );
        fileAppender->setName( LOG4CPLUS_TEXT( "file" ) );
        fileAppender->setLayout( std::unique_ptr< log4cplus::Layout >( new log4cplus::PatternLayout( pattern ) ) );
        //        qtAppender->setName( LOG4CPLUS_TEXT( "qt" ) );
        //        qtAppender->setLayout( std::unique_ptr<log4cplus::Layout>( new log4cplus::PatternLayout( pattern ) ) );
        logger.setLogLevel( log4cplus::INFO_LOG_LEVEL );
        //第4步：为Logger实例添加ConsoleAppender
        logger.addAppender( consoleAppender );
        logger.addAppender( fileAppender );
        //        logger.addAppender( qtAppender );
        //第5步：使用宏将日志输出
        LOG4CPLUS_INFO( logger, LOG4CPLUS_TEXT( "New Work Started!" ) );
        return true;
    }();
    ( void )temp;
    return logger;
}

//"%D{%m/%d/%y %H:%M:%S,%Q} [%t] %-5p: %m [%l]%n"
void ConsoleFileLogger::append( const log4cplus::spi::InternalLoggingEvent &event ) {
    const int      level = event.getLogLevel();
    const QString &str   = QString::fromStdWString(
                             log4cplus::helpers::getFormattedTime( L"%m/%d/%y %H:%M:%S,%Q", event.getTimestamp() ) ) +
                         QString( " [%1] " ).arg( QString::fromStdWString( event.getThread() ) ) +
                         getLogLevelText( level ) + ": " +
                         QString::fromStdWString( event.getMessage() ) +
                         QString( "[%1]\n" ).arg( QString::fromStdWString( event.getFunction() ) + ":" + QString::number( event.getLine() ) );
    emit write_something_signal( str, getColor( level ) );
}

QString ConsoleFileLogger::getLogLevelText( const int level ) {
    if( level == 10000 ) {
        return "DEBUG";
    }
    else if( level == 20000 ) {
        return "INFO ";
    }
    else if( level == 30000 ) {
        return "WARN ";
    }
    else if( level == 40000 ) {
        return "ERROR";
    }
    else if( level == 50000 ) {
        return "FATAL";
    }
}

QColor ConsoleFileLogger::getColor( const int level ) {
    if( level == 10000 ) {
        return Qt::blue;
    }
    else if( level == 20000 ) {
        return Qt::black;
    }
    else if( level == 30000 ) {
        return Qt::magenta;
    }
    else if( level == 40000 ) {
        return Qt::red;
    }
    else if( level == 50000 ) {
        return Qt::yellow;
    }
}
