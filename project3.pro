QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    RecognizeImg/inference.cpp \
    RecognizeImg/recognizeimgthread.cpp \
    WindowOne/ProTree/opentreethread.cpp \
    WindowOne/PicShow/picbutton.cpp \
    WindowOne/PicShow/picshow.cpp \
    WindowOne/ProTree/protree.cpp \
    WindowOne/ProTree/protreeitem.cpp \
    WindowOne/ProTree/protreewidget.cpp \
    WindowOne/ProTree/removeprodialog.cpp \
    WindowOne/windowone.cpp \
    WindowOne/PicDetection/picdetection.cpp\
    WindowTwo/windowtwo.cpp \
    WindowTwo/CameraThread/camerathread.cpp \
    WindowTwo/settingdialog.cpp



HEADERS += \
    const.h \
    mainwindow.h \
    RecognizeImg/inference.h \
    RecognizeImg/recognizeimgthread.h \
    WindowOne/ProTree/opentreethread.h \
    WindowOne/PicShow/picbutton.h \
    WindowOne/PicShow/picshow.h \
    WindowOne/ProTree/protree.h \
    WindowOne/ProTree/protreeitem.h \
    WindowOne/ProTree/protreewidget.h \
    WindowOne/ProTree/removeprodialog.h \
    WindowOne/windowone.h \
    WindowOne/PicDetection/picdetection.h \
    WindowTwo/windowtwo.h \
    WindowTwo/CameraThread/camerathread.h \
    WindowTwo/settingdialog.h

FORMS += \
    mainwindow.ui \
    WindowOne/PicShow/picshow.ui \
    WindowOne/ProTree/protree.ui \
    WindowOne/ProTree/removeprodialog.ui \
    WindowOne/windowone.ui \
    WindowOne/PicDetection/picdetection.ui \
    WindowTwo/windowtwo.ui \
    WindowTwo/settingdialog.ui

INCLUDEPATH += \
    $$PWD \
    $$PWD/WindowOne \
    $$PWD/RecognizeImg \
    $$PWD/WindowOne/ProTree \
    $$PWD/WindowOne/PicShow \
    $$PWD/WindowOne/PicDetection \
    $$PWD/WindowTwo \
    $$PWD/WindowTwo/CameraThread


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


# ---------------- OpenCV 配置 ----------------
# Windows Release 版本
CONFIG(release, debug|release): LIBS += -LE:/opencv/build/x64/vc16/lib/ -lopencv_world490
# Windows Debug 版本
else:CONFIG(debug, debug|release): LIBS += -LE:/opencv/build/x64/vc16/lib/ -lopencv_world490d

# OpenCV 包含路径和依赖路径
INCLUDEPATH += E:/opencv/build/include \    # 包含头文件目录
               E:/opencv/build/include/opencv2 \

DEPENDPATH += E:/opencv/build/include \
              E:/opencv/build/include/opencv2 \

# ---------------- ONNX Runtime 配置 ----------------
# Windows Release 版本
CONFIG(release, debug|release): LIBS += -LE:/onnxruntime/onnxruntime-win-x64-1.16.0/lib/ -lonnxruntime
# Windows Debug 版本
else:CONFIG(debug, debug|release): LIBS += -LE:/onnxruntime/onnxruntime-win-x64-1.16.0/lib/ -lonnxruntime

# ONNX Runtime 包含路径和依赖路径
INCLUDEPATH += E:/onnxruntime/onnxruntime-win-x64-1.16.0/include  # 头文件目录
DEPENDPATH += E:/onnxruntime/onnxruntime-win-x64-1.16.0/include     # 库文件目录

RESOURCES += \
    resource.qrc
