QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    opentreethread.cpp \
    picbutton.cpp \
    picshow.cpp \
    protree.cpp \
    protreeitem.cpp \
    protreewidget.cpp \
    removeprodialog.cpp \
    windowone.cpp \
    windowtwo.cpp

HEADERS += \
    const.h \
    mainwindow.h \
    opentreethread.h \
    picbutton.h \
    picshow.h \
    protree.h \
    protreeitem.h \
    protreewidget.h \
    removeprodialog.h \
    windowone.h \
    windowtwo.h

FORMS += \
    mainwindow.ui \
    picshow.ui \
    protree.ui \
    removeprodialog.ui \
    windowone.ui \
    windowtwo.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
