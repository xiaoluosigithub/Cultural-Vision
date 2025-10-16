#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>
#include <QStringList>
#include <QThread>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <QMessageBox>
#include <QFile>

namespace Ui {
class SettingDialog;
}

// 负责在后台枚举相机的工作线程类
class CameraEnumWorker : public QThread
{
    Q_OBJECT
public:
    explicit CameraEnumWorker(QObject *parent = nullptr);
    void run() override;

signals:
    void camerasEnumerated(const QStringList &cameraList);
};

class SettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent = nullptr);
    ~SettingDialog();
    int selectedCameraIndex() const { return m_selectedIndex; }

private slots:
    void on_btnOK_clicked();
    void on_btnCancel_clicked();
    void onCamerasEnumerated(const QStringList &cameraList);

private:
    Ui::SettingDialog *ui;
    int m_selectedIndex = 0;
    CameraEnumWorker *m_worker = nullptr;

    void startEnumerateCameras(); // 异步启动摄像头枚举
};

#endif // SETTINGDIALOG_H
