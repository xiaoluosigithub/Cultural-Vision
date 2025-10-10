#ifndef WINDOW_TWO_H
#define WINDOW_TWO_H

#include <QDialog>

namespace Ui {
class window_two;
}

class window_two : public QDialog
{
    Q_OBJECT

public:
    explicit window_two(QWidget *parent = nullptr);
    ~window_two();

private:
    Ui::window_two *ui;
signals:
    void window_two_Closed(window_two* window);
};

#endif // WINDOW_TWO_H
