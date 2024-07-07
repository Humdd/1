INCLUDE_INC_PATH = C:/3rdLibary/Include

INCLUDEPATH += $${INCLUDE_INC_PATH} $${INCLUDE_INC_PATH}/SupportClass

HEADERS += \
#        $${INCLUDE_INC_PATH}/ClassFactory.h \
#        $${INCLUDE_INC_PATH}/ClassFactory_Params.h \
        $${INCLUDE_INC_PATH}/ContainerOperation.h \
#        $${INCLUDE_INC_PATH}/DataStatistics.h \
#        $${INCLUDE_INC_PATH}/DataStorage.h \
        $${INCLUDE_INC_PATH}/DirManager.h \
#        $${INCLUDE_INC_PATH}/JsonHelper.h \
        $${INCLUDE_INC_PATH}/MyMacros.h \
        $${INCLUDE_INC_PATH}/Serialization.h \
#        $${INCLUDE_INC_PATH}/SingletonTemplate.h \
        $${INCLUDE_INC_PATH}/TimeRecorder.h \
        $${INCLUDE_INC_PATH}/SupportClass/fmtStr.h \
        $${INCLUDE_INC_PATH}/SupportClass/MyTypeTraits.h \
#        $${INCLUDE_INC_PATH}/SupportClass/MyMacro_Pack.h \
#        $${INCLUDE_INC_PATH}/ObjectGuard.h \
        $${INCLUDE_INC_PATH}/StringOperation.h

SOURCES += \
        $${INCLUDE_INC_PATH}/DirManager.cpp \
        $${INCLUDE_INC_PATH}/JsonHelper.cpp \
        $${INCLUDE_INC_PATH}/TimeRecorder.cpp \
        $${INCLUDE_INC_PATH}/DataStorage.cpp \
        $${INCLUDE_INC_PATH}/MyMacros.cpp \

#CONFIG(debug, debug|release){
#LIBS +=       C:/Program_Sources/Lib_Dll/Include/lib/JsonHelperd.lib \
#              C:/Program_Sources/Lib_Dll/Include/lib/DirManagerd.lib \

#} else {
#LIBS +=       C:/Program_Sources/Lib_Dll/Include/lib/JsonHelper.lib \
#              C:/Program_Sources/Lib_Dll/Include/lib/DirManager.lib \
#}
