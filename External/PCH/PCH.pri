CONFIG += precompile_header
INCLUDEPATH += $$PWD
PRECOMPILED_HEADER += $$PWD/stable.h
precompile_header:!isEmpty(PRECOMPILED_HEADER) {
DEFINES += USING_PCH
}