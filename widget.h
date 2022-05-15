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

public slots:
    void playnext();

private slots:
    void on_start_clicked();

    void on_stop_clicked();

    void on_resume_clicked();

    void on_close_clicked();

    void on_setspeed_clicked();

    void on_setpos_clicked();

    void on_setmode_clicked();

    void on_setVolume_clicked();

private:
    Ui::Widget *ui;
    AVPlayer* player = nullptr;
};
#endif // WIDGET_H
