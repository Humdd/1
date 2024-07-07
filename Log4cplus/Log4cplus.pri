LOG4CPLUS_PATH = $$PWD

INCLUDEPATH += \
    $${LOG4CPLUS_PATH} \
    $${LOG4CPLUS_PATH}/include \
    $${LOG4CPLUS_PATH}/include/log4cplus

CONFIG(debug, debug|release){
LIBS += -L$${LOG4CPLUS_PATH}/Debug/ -llog4cplusUD
} else {
LIBS += -L$${LOG4CPLUS_PATH}/Release/ -llog4cplusU
}

HEADERS += \
    $${LOG4CPLUS_PATH}/ConsoleFileLogger.h

SOURCES += \
    $${LOG4CPLUS_PATH}/ConsoleFileLogger.cpp
