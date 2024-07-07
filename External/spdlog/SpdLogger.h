//
// Created by TonyMa on 2022/8/15.
//

#ifndef CHIPTAGGINGTOOL_SPDLOGGER_H
#define CHIPTAGGINGTOOL_SPDLOGGER_H
//#define SPDLOGGER_INIT_ST
//#define SPDLOGGER_INIT_MT
#ifdef SPDLOGGER_INIT_ST || SPDLOGGER_INIT_MT
    #include "spdlog/spdlog.h"

/*！
 *  以下是一个封装了简单spdlog操作的类或宏，要使用spdlog操作，请在项目配置文件（pro、cmake等）中定义宏：
 *  SPDLOGGER_INIT_ST ： 使用单线程版的SpdLogger类
 *  SPDLOGGER_INIT_MT ： 使用多线程版的SpdLogger类
 *  定义好宏后，请全程使用对应宏来操作SpdLog，请不要使用SpdLogger类
 */
struct SpdLogger {
    inline static bool           isExcept = false;
    inline static spdlog::logger trace_logger{ "" };

private:
    struct SpdLoggerPrivate {
        SpdLoggerPrivate( const std::function< std::vector< spdlog::sink_ptr >( void ) > &func ) noexcept;

        ~SpdLoggerPrivate();
    };
    struct SpdLoggerIniter {
        SpdLoggerIniter() noexcept {
    #ifdef SPDLOGGER_INIT_ST
            SpdLogger::initLogger_st();
    #elif SPDLOGGER_INIT_MT
            SpdLogger::initLogger_mtt();
    #endif
        }
    };

    static void                              initLogger_st() noexcept;//初始化单线程版的Logger
    static void                              initLogger_mt() noexcept;//初始化多线程版的Logger
    inline static SpdLogger::SpdLoggerIniter spdloger_initer;
    friend struct SpdLoggerPrivate;
    friend struct SpdLoggerIniter;
};

    #define SPDLOG_EXIT_AND_OUTPUT( message )                                             \
        SPDLOG_CRITICAL( message );                                                       \
        SpdLogger::trace_logger.dump_backtrace();                                         \
        SPDLOG_CRITICAL( "End : =============================================== : End" ); \
        SpdLogger::isExcept = true;                                                       \
        spdlog::drop_all();                                                               \
        exit( 1 );

    #define SPDLOG_ASSERT( condition )                                                       \
        if( !( condition ) ) {                                                               \
            SPDLOG_EXIT_AND_OUTPUT( "The Assert is UnEstablished : { " #condition " } !!!" ) \
        }

    #define SPDLOG_EXIT_AND_OUTPUT_IF( condition, message ) \
        if( !( condition ) ) {                              \
            SPDLOG_EXIT_AND_OUTPUT( message )               \
        }

    #define SPDLOG_INFO_IF( condition, text ) \
        if( condition ) { SPDLOG_INFO( text ); }

    #define SPDLOG_DEBUG_IF( condition, text ) \
        if( condition ) { SPDLOG_DEBUG( text ); }

    #define SPDLOG_WARN_IF( condition, text ) \
        if( condition ) { SPDLOG_WARN( text ); }

    #define SPDLOG_ERROR_IF( condition, text ) \
        if( condition ) { SPDLOG_ERROR( text ); }

    #define SPDLOG_CRITICAL_IF( condition, text ) \
        if( condition ) { SPDLOG_CRITICAL( text ); }

    #define SPDLOG_TRACE_FUNC() SpdLogger::trace_logger.trace( "We arrived in {} at file://{}:{}", __FUNCTION__, __FILE__, __LINE__ )

    //void ExceptionHandler() noexcept; //统一处理异常的函数，具体处理方法是打日志和强制退出程序

    #include "TimeRecorder.h"

struct SpdlogTimerRecordGuard: protected TimeRecordGuard {
    using TimeRecordGuard::TimeRecordGuard;

    ~SpdlogTimerRecordGuard() {
        SPDLOG_INFO( m_timer.outputContent( doing_thing_str ).toStdString() );
    }
};

    #define SPDLOG_TIME_RECORD_GUARD SpdlogTimerRecordGuard spdlog_time_record_guard( TIME_RECORD_OUTPUT( "Test Function Time" ) )

#endif//SPDLOGGER_INIT_ST || SPDLOGGER_INIT_MT
#endif//CHIPTAGGINGTOOL_SPDLOGGER_H
