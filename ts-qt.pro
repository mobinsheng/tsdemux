TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += \
    libtsdemux

SOURCES +=  \
    libtsdemux/cat.c \
    libtsdemux/packet.c \
    libtsdemux/pat.c \
    libtsdemux/pid.c \
    libtsdemux/pmt.c \
    libtsdemux/stream.c \
    libtsdemux/table.c \
    libtsdemux/types.c \
    tsdump/tsdump.c

HEADERS += \
    libtsdemux/p_tsdemux.h \
    libtsdemux/tsdemux.h

