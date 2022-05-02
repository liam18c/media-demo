#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include "avplayer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();


private slots:
    void on_pushButton_clicked();

    void on_exit_clicked();

    void on_resume_clicked();

    void on_show_clicked();

private:
    Ui::Widget *ui;
    AVPlayer* player = nullptr;
};
#endif // WIDGET_H
