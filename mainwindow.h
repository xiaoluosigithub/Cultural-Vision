// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "windowone.h"
#include "windowtwo.h"
#include "const.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

extern QString LABEL_PATH;
extern QString MODEL_PATH;

class WindowOne;   // 前向声明
class WindowTwo;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openWindowOne();
    void openWindowTwo();
    void windowOneClosed();
    void windowTwoClosed();

private:
    void disableMainButtons();
    void enableMainButtons();

private:
    Ui::MainWindow *ui;
    WindowOne* windowOne;
    WindowTwo* windowTwo;

    // 将资源写到磁盘的函数
    QString copyResourceToDisk(const QString &resPath, const QString &targetFileName);
    virtual void resizeEvent(QResizeEvent *event);
};

#endif // MAINWINDOW_H
