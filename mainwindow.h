// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "windowone.h"
#include "windowtwo.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

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
    WindowOne windowOne;
    WindowTwo windowTwo;
};

#endif // MAINWINDOW_H
