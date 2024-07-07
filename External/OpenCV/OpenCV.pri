OpenCV_Path = C:/3rdLibary/OpenCV

INCLUDEPATH += $$PWD \
               $$PWD/SupportClass \
               $${OpenCV_Path}/include \
               $${OpenCV_Path}/include/opencv2
DEPENDPATH += $$PWD \
                $$PWD/SupportClass \
                $${OpenCV_Path}/include \
                $${OpenCV_Path}/include/opencv2

OpenCV_Lib_Path = $${OpenCV_Path}/x64/vc16/lib

CONFIG(debug, debug|release){

LIBS +=       $${OpenCV_Lib_Path}/opencv_world454d.lib \
              $${OpenCV_Lib_Path}/opencv_img_hash454d.lib \
#              C:/Program_Sources/Lib_Dll/Include/lib/OpenCVHelpd.lib \
#              C:/Program_Sources/Lib_Dll/Include/lib/Cv2Qtd.lib \
} else {

LIBS +=       $${OpenCV_Lib_Path}/opencv_world454.lib \
              $${OpenCV_Lib_Path}/opencv_img_hash454.lib \
#              C:/Program_Sources/Lib_Dll/Include/lib/OpenCVHelp.lib \
#              C:/Program_Sources/Lib_Dll/Include/lib/Cv2Qt.lib \
}
	
HEADERS += \
     $$PWD/OpenCVSerialization.h \
     $$PWD/SupportClass/Cv2Qt.h \
     $$PWD/OpenCVOutput.h \
     $$PWD/OpenCVHelp.h \
     $$PWD/SupportClass/Rotater.h

SOURCES += \
#    $$PWD/OpenCVSerialization.cpp \
    $$PWD/SupportClass/Cv2Qt.cpp \
    $$PWD/OpenCVHelp.cpp \
#    $$PWD/OpenCVOutput.cpp \
    $$PWD/SupportClass/Rotater.cpp

