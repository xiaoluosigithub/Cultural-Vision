QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    WindowOne/ProTree/opentreethread.cpp \
    WindowOne/PicShow/picbutton.cpp \
    WindowOne/PicShow/picshow.cpp \
    WindowOne/ProTree/protree.cpp \
    WindowOne/ProTree/protreeitem.cpp \
    WindowOne/ProTree/protreewidget.cpp \
    WindowOne/ProTree/removeprodialog.cpp \
    WindowOne/windowone.cpp \
    WindowOne/PicDetection/picdetection.cpp\
    WindowTwo/windowtwo.cpp


HEADERS += \
    const.h \
    mainwindow.h \
    WindowOne/ProTree/opentreethread.h \
    WindowOne/PicShow/picbutton.h \
    WindowOne/PicShow/picshow.h \
    WindowOne/ProTree/protree.h \
    WindowOne/ProTree/protreeitem.h \
    WindowOne/ProTree/protreewidget.h \
    WindowOne/ProTree/removeprodialog.h \
    WindowOne/windowone.h \
    WindowOne/PicDetection/picdetection.h \
    WindowTwo/windowtwo.h


FORMS += \
    mainwindow.ui \
    WindowOne/PicShow/picshow.ui \
    WindowOne/ProTree/protree.ui \
    WindowOne/ProTree/removeprodialog.ui \
    WindowOne/windowone.ui \
    WindowOne/PicDetection/picdetection.ui \
    WindowTwo/windowtwo.ui

INCLUDEPATH += \
    $$PWD \
    $$PWD/WindowOne \
    $$PWD/WindowOne/ProTree \
    $$PWD/WindowOne/PicShow \
    $$PWD/WindowOne/PicDetection \
    $$PWD/WindowTwo


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
