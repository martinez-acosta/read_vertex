TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    cmdline.c \
    raster_lines.c

HEADERS += \
    cmdline.h \
    definiciones.h \
    raster_lines.h
