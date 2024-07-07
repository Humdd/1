spdlog_Path = C:/3rdLibary/spdlog
INCLUDEPATH += $$PWD \
                $${spdlog_Path}/include \
                $${spdlog_Path}/spdlog \
                $${spdlog_Path}/spdlog/include/spdlog

#CONFIG(debug, debug|release){

#LIBS +=       $${Spdlog_Lib_Path}/fmtd.lib \
#              $${Spdlog_Lib_Path}/spdlogd.lib
#} else {

#LIBS +=       $${Spdlog_Lib_Path}/fmt.lib \
#              $${Spdlog_Lib_Path}/spdlog.lib
#}

HEADERS += \
#         $$PWD/EncryptedRotatingFileSink.h \
#         $$PWD/QtSink.h \
         $$PWD/SpdLogger.h

SOURCES += \
#        $$PWD/QtSink.cpp \
        $$PWD/SpdLogger.cpp

#CONFIG(debug, debug|release){
#LIBS +=       C:/Program_Sources/Lib_Dll/Include/lib/SpdLoggerd.lib \

#} else {
#LIBS +=       C:/Program_Sources/Lib_Dll/Include/lib/SpdLogger.lib \

#}
