#ifndef PROTREEWIDGET_H
#define PROTREEWIDGET_H

#include <QObject>
#include <QTreeWidget>
#include <QProgressDialog>
#include <QString>
#include <QDir>
#include "opentreethread.h"
#include "const.h"


class ProTreeWidget : public QTreeWidget
{
    Q_OBJECT
private:
    QSet<QString> _set_path;
    QProgressDialog * _dialog_progress;
    QProgressDialog * _open_progressdlg;
    std::shared_ptr<OpenTreeThread> _thread_open_pro;
public:
    ProTreeWidget(QWidget *parent = nullptr);
public slots:
    void SlotOpenPro(const QString&  path);
private slots:
    void SlotUpOpenProgress(int count);
    void SlotFinishOpenProgress();
    void SlotCancelOpenProgress();
signals:
    void SigCancelOpenProgress();
};

#endif // PROTREEWIDGET_H
