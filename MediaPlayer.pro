#-------------------------------------------------
#
# Project created by QtCreator 2013-09-22T05:07:03
#
#-------------------------------------------------

TEMPLATE = app
QT      += multimedia gui
TARGET   = MediaPlayer

SOURCES += main.cpp \
		videoplayer.cpp \
		videowidget.cpp \
		videowidgetsurface.cpp \
		FFmpegMovie.cpp

HEADERS += videoplayer.h \
		videowidget.h \
		videowidgetsurface.h \
		FFmpegMovie.h

INCLUDEPATH += /usr/include/QtMobility 
FORMS    += videoplayer.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../usr/lib/release/
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../usr/lib/debug/
else:symbian: LIBS +=
else:unix: LIBS += -lavcodec -lavutil -lavformat -lswscale -lavfilter

