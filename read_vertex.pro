TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    cmdline.c \
    definiciones.c \
    draw_lines.c

HEADERS += \
    cmdline.h \
    definiciones.h \
    draw_lines.h
