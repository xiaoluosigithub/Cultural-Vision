// mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"

QString MODEL_PATH;
QString LABEL_PATH;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("刺绣检测");  // 设置窗口标题
    this->setWindowIcon(QIcon(":/icon/main_img.png"));  // 设置窗口图标

    QPixmap pixmap(":/icon/mainImg2.png");
    QSize parentSize = ui->img_label->parentWidget()->size(); // 父控件大小

    ui->img_label->setPixmap(pixmap.scaled(
        parentSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        ));


    // 打开样式文件（这里假设 qss 文件放在资源文件中）
    QFile qssFile(":/style/MainWindow.qss");

    // 检查文件是否成功打开（只读方式）
    if (qssFile.open(QFile::ReadOnly)) {

        // 读取整个 QSS 文件内容，并将其转换为 QString
        QString style = QLatin1String(qssFile.readAll());

        // 将样式表应用到当前对话框（this 指向 RemoveProDialog）
        this->setStyleSheet(style);

        // 关闭文件
        qssFile.close();
    }


    MODEL_PATH = copyResourceToDisk(DEF_MODEL_PATH, "best.onnx");
    LABEL_PATH = copyResourceToDisk(DEF_LABEL_PATH, "class_names.txt");

    if (MODEL_PATH.isEmpty() || LABEL_PATH.isEmpty()) {
        qWarning() << "Failed to write resource files to disk!";
    } else {
        qDebug() << "Resources ready on disk:";
        qDebug() << "Model:" << MODEL_PATH;
        qDebug() << "Label:" << LABEL_PATH;
    }

    // MODEL_PATH = g_modelPathOnDisk;
    // LABEL_PATH = g_labelPathOnDisk;

    // 初始化窗口对象
    windowOne = new WindowOne(this);
    windowTwo = new WindowTwo(this);

    // 连接主界面按钮
    connect(ui->pushButton1, &QPushButton::clicked, this, &MainWindow::openWindowOne);
    connect(ui->pushButton2, &QPushButton::clicked, this, &MainWindow::openWindowTwo);

    // 连接副界面关闭信号
    connect(windowOne, &WindowOne::windowClosed, this, &MainWindow::windowOneClosed);
    connect(windowTwo, &WindowTwo::windowClosed, this, &MainWindow::windowTwoClosed);



    QIcon icon1(":/icon/11.png");
    ui->pushButton1->setIcon(icon1);
    ui->pushButton1->setIconSize(ui->pushButton1->size()); // 初始设置

    QIcon icon2(":/icon/12.png");
    ui->pushButton2->setIcon(icon2);
    ui->pushButton2->setIconSize(ui->pushButton2->size()); // 初始设置


    // 延迟初始化，让布局完成后再设置图标大小和 label pixmap
    QTimer::singleShot(0, this, [=](){
        // 更新按钮图标大小
        ui->pushButton1->setIconSize(ui->pushButton1->size());
        ui->pushButton2->setIconSize(ui->pushButton2->size());
    });
}

MainWindow::~MainWindow()
{
    delete windowOne;
    delete windowTwo;
    delete ui;
}

void MainWindow::disableMainButtons()
{
    ui->pushButton1->setEnabled(false);
    ui->pushButton2->setEnabled(false);
}

void MainWindow::enableMainButtons()
{
    ui->pushButton1->setEnabled(true);
    ui->pushButton2->setEnabled(true);
}

QString MainWindow::copyResourceToDisk(const QString &resPath, const QString &targetFileName)
{
    // 临时目录
    QString targetPath = QDir::tempPath() + "/" + targetFileName;

    // 如果文件已存在，先删除
    if (QFile::exists(targetPath)) {
        QFile::remove(targetPath);
    }

    // 打开资源文件
    QFile resFile(resPath);
    if (!resFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open resource:" << resPath;
        return QString();
    }

    // 打开磁盘文件准备写入
    QFile outFile(targetPath);
    if (!outFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot write file:" << targetPath;
        resFile.close();
        return QString();
    }

    // 复制内容
    outFile.write(resFile.readAll());

    resFile.close();
    outFile.close();

    qDebug() << "Resource" << resPath << "written to disk:" << targetPath;

    return targetPath;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event); // 保持默认行为

    if (ui->pushButton1) ui->pushButton1->setIconSize(ui->pushButton1->size());
    if (ui->pushButton2) ui->pushButton2->setIconSize(ui->pushButton2->size());

    if (!ui->img_label->pixmap(Qt::ReturnByValue).isNull()) {
        QPixmap pix = QPixmap(":/icon/mainImg2.png"); // 原始图像
        ui->img_label->setPixmap(pix.scaled(
            ui->img_label->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            ));
    }
}

void MainWindow::openWindowOne()
{
    disableMainButtons();
    if (!windowOne->isVisible()) {
        windowOne->show();
    } else {
        windowOne->raise();
        windowOne->activateWindow();
    }
}

void MainWindow::openWindowTwo()
{
    disableMainButtons();
    if (!windowTwo->isVisible()) {
        windowTwo->show();
    } else {
        windowTwo->raise();
        windowTwo->activateWindow();
    }
}

void MainWindow::windowOneClosed()
{
    enableMainButtons();
}

void MainWindow::windowTwoClosed()
{
    enableMainButtons();
}
