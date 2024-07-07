//
// Created by TonyMa on 2022/8/15.
//
#include "QtSink.h"
#include <QDebug>
#include <QMap>

void MsgSender::emitSignal( const std::string msg, const int
                                                      #ifdef QT_USER_NEED
                                                      level
                                                      #endif
                           ) const noexcept {
    #ifdef QT_USER_NEED
    emit send_message( QString::fromStdString( msg ), getColor( level ) );
    #endif
}

const QColor &MsgSender::getColor( const int level ) noexcept {
    static const QMap<int, QColor> dst = {
        { SPDLOG_LEVEL_TRACE, Qt::black },
        { SPDLOG_LEVEL_DEBUG, Qt::blue },
        { SPDLOG_LEVEL_INFO, Qt::green },
        { SPDLOG_LEVEL_WARN, Qt::yellow },
        { SPDLOG_LEVEL_ERROR, Qt::red },
        { SPDLOG_LEVEL_CRITICAL, Qt::darkRed }
    };
    assert( dst.contains( level ) );
    return dst[level];
}
