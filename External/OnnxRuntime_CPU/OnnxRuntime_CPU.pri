INCLUDEPATH += $$PWD/onnxruntime-win-x64-1.15.1/include

LIBS += $$PWD/onnxruntime-win-x64-1.15.1/lib/onnxruntime.lib

HEADERS += \
    $$PWD/onnxruntime_cpu.h

SOURCES += \
    $$PWD/onnxruntime_cpu.cpp
