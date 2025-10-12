#include "settingdialog.h"
#include "ui_settingdialog.h"
#include <opencv2/opencv.hpp>
#include <QMessageBox>

/* ---------------- 摄像头枚举线程实现 ---------------- */
CameraEnumWorker::CameraEnumWorker(QObject *parent)
    : QThread(parent)
{
}

void CameraEnumWorker::run()
{
    QStringList cameraList;
    for (int i = 0; i < 5; ++i) {
        cv::VideoCapture cap(i);
        if (cap.isOpened()) {
            cameraList << QString("摄像头 %1").arg(i);
            cap.release();
        }
    }
    if (cameraList.isEmpty()) {
        cameraList << "未检测到摄像头";
    }
    emit camerasEnumerated(cameraList);
}

/* ---------------- 设置界面实现 ---------------- */
SettingDialog::SettingDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingDialog)
{
    ui->setupUi(this);

    connect(ui->btnOK, &QPushButton::clicked, this, &SettingDialog::on_btnOK_clicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &SettingDialog::on_btnCancel_clicked);

    // UI 初始状态
    ui->comboBoxCamera->addItem("正在检测摄像头...");
    startEnumerateCameras();
}

SettingDialog::~SettingDialog()
{
    if (m_worker && m_worker->isRunning()) {
        m_worker->quit();
        m_worker->wait();
    }
    delete ui;
}

// 点击“确定”
void SettingDialog::on_btnOK_clicked()
{
    if (ui->comboBoxCamera->count() > 0) {
        m_selectedIndex = ui->comboBoxCamera->currentIndex();
    } else {
        m_selectedIndex = 0;
    }
    accept();
}

// 点击“取消”
void SettingDialog::on_btnCancel_clicked()
{
    reject();
}

// 启动异步摄像头枚举线程
void SettingDialog::startEnumerateCameras()
{
    if (m_worker) {
        m_worker->deleteLater();
    }

    m_worker = new CameraEnumWorker(this);
    connect(m_worker, &CameraEnumWorker::camerasEnumerated, this, &SettingDialog::onCamerasEnumerated);
    m_worker->start();
}

// 当后台线程返回摄像头列表时更新UI
void SettingDialog::onCamerasEnumerated(const QStringList &cameraList)
{
    ui->comboBoxCamera->clear();
    ui->comboBoxCamera->addItems(cameraList);
}
