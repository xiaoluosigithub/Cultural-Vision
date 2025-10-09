// window_one.h
#ifndef WINDOW_ONE_H
#define WINDOW_ONE_H

#include <QDialog>
#include <QWidget>
#include "protree.h"
#include <QFileDialog>
#include <QTreeWidget>
#include "protreewidget.h"
#include "picshow.h"

namespace Ui { class WindowOne; }

class WindowOne : public QDialog
{
    Q_OBJECT

public:
    explicit WindowOne(QWidget *parent = nullptr);
    ~WindowOne();

signals:
    void windowClosed();
    void SigOpenPro(const QString &path);
private slots:
    void handleClose();
    void SlotOpenPro(bool);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::WindowOne *ui;
    QWidget * _protree;
    QWidget * _picshow;
};

#endif // WINDOW_ONE_H
