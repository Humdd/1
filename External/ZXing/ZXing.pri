ZXING_PATH = C:/3rdLibary/ZXing

INCLUDEPATH += $${ZXING_PATH}/src \
                $${ZXING_PATH}/example

HEADERS += $${ZXING_PATH}/example/ZXingOpenCV.h

CONFIG(debug, debug|release){
LIBS +=      $${ZXING_PATH}/build/core/Debug/ZXing.lib
} else {
LIBS +=      $${ZXING_PATH}/build/core/Release/ZXing.lib
}

