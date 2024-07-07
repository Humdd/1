#ifndef CONSOLEFILELOGGER_H
#define CONSOLEFILELOGGER_H
#include "Log4cplus/include/log4cplus/log4cplus.h"
#include "TimeRecorder.h"
#include <QObject>
#include <QColor>

class ConsoleFileLogger: public QObject
    , public log4cplus::Appender {
    Q_OBJECT;

public:
    static ConsoleFileLogger *getInstance();
    // Appender interface
    virtual void        close() override;
    static std::wstring logContent( const QString &str );
public slots:
    void timingStart();

    void lastingTime( const QString &str );

signals:
    void write_something_signal( const QString &str, const QColor &color );

protected:
    ConsoleFileLogger( QObject *parent = nullptr );
    ~ConsoleFileLogger();
    // Appender interface
    virtual void   append( const log4cplus::spi::InternalLoggingEvent &event ) override;
    static QString getLogLevelText( const int level );
    QColor         getColor( const int level );
    TimeRecorder   m_timer;
};
struct GetLogger {
    static log4cplus::Logger &getLogger();
};

static log4cplus::Logger &_logger = GetLogger::getLogger();

#define LOG4CPLUS_INFO_EXIT( _Text )                                                                                                                                 \
    LOG4CPLUS_INFO( _logger, ConsoleFileLogger::logContent( _Text ) );                                                                                               \
    _logger.getAppender( LOG4CPLUS_TEXT( "file" ) )->setLayout( std::unique_ptr< log4cplus::Layout >( new log4cplus::PatternLayout( LOG4CPLUS_TEXT( "%n" ) ) ) );    \
    _logger.getAppender( LOG4CPLUS_TEXT( "console" ) )->setLayout( std::unique_ptr< log4cplus::Layout >( new log4cplus::PatternLayout( LOG4CPLUS_TEXT( "%n" ) ) ) ); \
    LOG4CPLUS_INFO( _logger, LOG4CPLUS_TEXT( "" ) );

#define LOG4CPLUS_ASSERT_WILL_EXIT( _logger, condition )                                                                                                                 \
    LOG4CPLUS_ASSERT( _logger, condition );                                                                                                                              \
    if( !( condition ) ) {                                                                                                                                               \
        LOG4CPLUS_FATAL( _logger, LOG4CPLUS_TEXT( "Program Exit Abnormally!" ) );                                                                                        \
        _logger.getAppender( LOG4CPLUS_TEXT( "file" ) )->setLayout( std::unique_ptr< log4cplus::Layout >( new log4cplus::PatternLayout( LOG4CPLUS_TEXT( "%n" ) ) ) );    \
        _logger.getAppender( LOG4CPLUS_TEXT( "console" ) )->setLayout( std::unique_ptr< log4cplus::Layout >( new log4cplus::PatternLayout( LOG4CPLUS_TEXT( "%n" ) ) ) ); \
        LOG4CPLUS_INFO( _logger, LOG4CPLUS_TEXT( "" ) );                                                                                                                 \
        exit( 3 );                                                                                                                                                       \
    }

#define Q_LOG4CPLUS_DEBUG( _Text ) \
    LOG4CPLUS_DEBUG( _logger, ConsoleFileLogger::logContent( _Text ) );

#define Q_LOG4CPLUS_INFO( _Text ) \
    LOG4CPLUS_INFO( _logger, ConsoleFileLogger::logContent( _Text ) );

#define Q_LOG4CPLUS_WARN( _Text ) \
    LOG4CPLUS_WARN( _logger, ConsoleFileLogger::logContent( _Text ) );

#define Q_LOG4CPLUS_ERROR( _Text ) \
    LOG4CPLUS_ERROR( _logger, ConsoleFileLogger::logContent( _Text ) );

#define Q_LOG4CPLUS_FATAL( _Text ) \
    LOG4CPLUS_FATAL( _logger, ConsoleFileLogger::logContent( _Text ) );

#define Q_LOG4CPLUS_ASSERT( _condition ) \
    LOG4CPLUS_ASSERT_WILL_EXIT( _logger, _condition );

#define Q_LOG4CPLUS_TimingStart \
    ConsoleFileLogger::getInstance()->timingStart();

#define Q_LOG4CPLUS_TimingEnd \
    ConsoleFileLogger::getInstance()->lastingTime( __FUNCSIG__ );
//
struct TimeRecorderHelper_WithNote {
    TimeRecorderHelper_WithNote( const QString &m_str = "Do all the things" )
        : m_str( m_str ) {}
    ~TimeRecorderHelper_WithNote() {
        Q_LOG4CPLUS_INFO( m_timer.outputContent( m_str ) );
    }

private:
    QString      m_str;
    TimeRecorder m_timer;
};
#define NoteTimingStart TimeRecorderHelper_WithNote time_recorder_helper_with_note(TIMER_RECORDER_OUTPUT( __FUNCSIG__ ));
#endif// CONSOLEFILELOGGER_H
