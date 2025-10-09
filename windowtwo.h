// window_two.h
#ifndef WINDOW_TWO_H
#define WINDOW_TWO_H

#include <QDialog>

namespace Ui { class WindowTwo; }

class WindowTwo : public QDialog
{
    Q_OBJECT

public:
    explicit WindowTwo(QWidget *parent = nullptr);
    ~WindowTwo();

signals:
    void windowClosed();

private slots:
    void handleClose();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::WindowTwo *ui;
};

#endif // WINDOW_TWO_H
