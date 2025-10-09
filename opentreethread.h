#ifndef OPENTREETHREAD_H
#define OPENTREETHREAD_H

#include <QThread>
#include <QTreeWidget>

class OpenTreeThread : public QThread
{
    Q_OBJECT
public:
    explicit OpenTreeThread(const QString& src_path, int file_count,
                            QTreeWidget *self, QObject *parent = nullptr);

    // 加载项目目录树
    void OpenProTree(const QString& src_path, int &file_count, QTreeWidget* self);

protected:
    void run() override;

private:
    // 递归加载目录
    void RecursiveProTree(const QString& src_path, int &file_count, QTreeWidgetItem* parent);

    QString _src_path;      // 根目录路径
    int _file_count;        // 文件计数
    QTreeWidget* _self;     // 目标树控件
    bool _bstop;            // 是否中止标志

signals:
    void SigFinishProgress(int);   // 加载完成信号
    void SigUpdateProgress(int);   // 进度更新信号

public slots:
    void SlotCancelProgress();     // 用户中止加载
};

#endif // OPENTREETHREAD_H
