#ifndef WINDOW_ONE_H
#define WINDOW_ONE_H

#include <QDialog>

namespace Ui {
class window_one;
}

class window_one : public QDialog
{
    Q_OBJECT

public:
    explicit window_one(QWidget *parent = nullptr);
    ~window_one();

private:
    Ui::window_one *ui;
signals:
    void window_one_Closed(window_one* window);
};

#endif // WINDOW_ONE_H
