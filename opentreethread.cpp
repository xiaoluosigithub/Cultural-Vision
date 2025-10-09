#include "opentreethread.h"
#include <QDir>
#include <QFileInfoList>
#include <QIcon>

OpenTreeThread::OpenTreeThread(const QString &src_path, int file_count,
                               QTreeWidget *self, QObject *parent)
    : QThread(parent),
    _src_path(src_path),
    _file_count(file_count),
    _self(self),
    _bstop(false)
{
}

void OpenTreeThread::OpenProTree(const QString &src_path, int &file_count, QTreeWidget *self)
{
    QDir src_dir(src_path);
    QString name = src_dir.dirName();

    // 创建根节点
    auto *rootItem = new QTreeWidgetItem(self);
    rootItem->setText(0, name);
    rootItem->setIcon(0, QIcon(":/icon/dir.png"));
    rootItem->setToolTip(0, src_path);

    // 递归加载
    RecursiveProTree(src_path, file_count, rootItem);
}

void OpenTreeThread::run()
{
    OpenProTree(_src_path, _file_count, _self);

    if (_bstop)
        return;

    emit SigFinishProgress(_file_count);
}

void OpenTreeThread::RecursiveProTree(const QString &src_path, int &file_count, QTreeWidgetItem *parent)
{
    QDir src_dir(src_path);
    src_dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    src_dir.setSorting(QDir::Name);
    QFileInfoList list = src_dir.entryInfoList();

    for (const QFileInfo &fileInfo : std::as_const(list))
    {
        if (_bstop)
            return;

        if (fileInfo.isDir())
        {
            file_count++;
            emit SigUpdateProgress(file_count);

            auto *item = new QTreeWidgetItem(parent);
            item->setText(0, fileInfo.fileName());
            item->setIcon(0, QIcon(":/icon/dir.png"));
            item->setToolTip(0, fileInfo.absoluteFilePath());

            // 递归进入子目录
            RecursiveProTree(fileInfo.absoluteFilePath(), file_count, item);
        }
        else
        {
            QString suffix = fileInfo.completeSuffix().toLower();
            if (suffix != "png" && suffix != "jpeg" && suffix != "jpg")
                continue;

            file_count++;
            emit SigUpdateProgress(file_count);

            auto *item = new QTreeWidgetItem(parent);
            item->setText(0, fileInfo.fileName());
            item->setIcon(0, QIcon(":/icon/pic.png"));
            item->setToolTip(0, fileInfo.absoluteFilePath());
        }
    }
}

void OpenTreeThread::SlotCancelProgress()
{
    _bstop = true;
}
