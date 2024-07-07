//
// Created by TonyMa on 2022/8/15.
//
#include "SpdLogger.h"
#include "MyMacros.h"

#ifdef SPDLOGGER_INIT_ST || SPDLOGGER_INIT_MT
//#include "EncryptedRotatingFileSink.h"
#ifdef NEED_QTSINK
    #include "QtSink.h"
#endif

#ifndef QT_NO_DEBUG_OUTPUT

    #include "spdlog/sinks/stdout_sinks.h"

#endif

#include "spdlog/sinks/rotating_file_sink.h"
#include "MyMacros.h"

void SpdLogger::initLogger_st() noexcept {
    static SpdLogger::SpdLoggerPrivate dst( []() {
        return std::vector< spdlog::sink_ptr >{
                std::make_shared< spdlog::sinks::rotating_file_sink_st >( "./log/ChipTaggingTool.log" ,
                        1024 * 1024 * 200 , 5 )
                onlyCanOutput( , std::make_shared< spdlog::sinks::stdout_sink_st >() )
        };
    } );
}

void SpdLogger::initLogger_mt() noexcept {
    static SpdLogger::SpdLoggerPrivate dst( []() {
        return std::vector< spdlog::sink_ptr >{
                std::make_shared< spdlog::sinks::rotating_file_sink_mt >( "./log/ChipTaggingTool.log" ,
                        1024 * 1024 * 200 , 5 )
                onlyCanOutput( , std::make_shared< spdlog::sinks::stdout_sink_mt >() )
        };
    } );
}

#define LOGGER_OUTPUT_IN_START_AND_END
#ifdef LOGGER_OUTPUT_IN_START_AND_END
    #define onlyInOUTPUT_IN_START_AND_END( ... ) MACRO_NO_CHANGED( __VA_ARGS__ )
#else
    #define onlyInOUTPUT_IN_START_AND_END( ... )
#endif

//构造时完成SpdLogger的初始化
SpdLogger::SpdLoggerPrivate::SpdLoggerPrivate(
        const std::function< std::vector< spdlog::sink_ptr >() > &func ) noexcept {
    auto &&sinks = func();
    auto &&logger = std::make_shared< spdlog::logger >( "logger" , sinks.begin() , sinks.end() );
    logger->flush_on( spdlog::level::trace );
    sinks[ 0 ]->set_level( spdlog::level::trace );
#ifdef NEED_QTSINK
    sinks[ 1 ]->set_level( spdlog::level::info );
#endif
#ifdef _DEBUG
    #ifdef NEED_QTSINK
    sinks[ 2 ]->set_level( spdlog::level::trace );
    #else
    sinks[ 1 ]->set_level( spdlog::level::trace );
    #endif
#endif
    spdlog::set_default_logger( logger );
    trace_logger = spdlog::logger( "trace_logger" , sinks.begin() , sinks.end() );
    trace_logger.enable_backtrace( 32 );//trace_logger只追踪最新的32个程序函数调用
    spdlog::default_logger()->set_pattern( "[%Y-%m-%d %H:%M:%S.%e][%l] %v" );
    onlyInOUTPUT_IN_START_AND_END( spdlog::default_logger()->info( "Program Start!!!" ); )
    spdlog::default_logger()
            ->set_pattern( "[%Y-%m-%d %H:%M:%S.%e][%l] %v [file://%@]" );//%@
    //处理不抛出异常的特殊奔溃事件的处理函数，主要是打日志（回溯Call Back）、退出程序
    SetUnhandledExceptionFilter( []( PEXCEPTION_POINTERS ) -> LONG {
        spdlog::default_logger()->set_pattern( "[%Y-%m-%d %H:%M:%S.%e][%l] %v" );
        SPDLOG_CRITICAL( "An Unknowed exception was happened!!!" );
        trace_logger.dump_backtrace();
        SPDLOG_CRITICAL( "End : =============================================== : End" );
        return EXCEPTION_EXECUTE_HANDLER;
    } );
}

//析构时打日志，标志一次程序的退出
SpdLogger::SpdLoggerPrivate::~SpdLoggerPrivate() {
    onlyInOUTPUT_IN_START_AND_END(
            if( !SpdLogger::isExcept ) {
                spdlog::default_logger()->set_pattern( "[%Y-%m-%d %H:%M:%S.%e][%l] %v" );
                SPDLOG_INFO( "Program End Succeed!!!" );
                SPDLOG_INFO( "End :====================: End" );
            } )
}
//统一处理异常的函数，具体处理方法是打日志和强制退出程序
//void ExceptionHandler() noexcept {
//    try {
//        throw; // re-throw exception already in flight
//    }
//    catch ( const QString &e ) {
//        SPDLOG_EXIT_AND_OUTPUT( e.toStdString() );
//    }
//    catch( const std::exception &e ) {
//        SPDLOG_EXIT_AND_OUTPUT( e.what() );
//    }
//    catch( ... ) {
//        SPDLOG_EXIT_AND_OUTPUT( "An Unknowed exception was caught!!!" );
//    }
//}
#endif