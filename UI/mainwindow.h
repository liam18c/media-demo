#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "player.h"

#include <QMainWindow>
#include <QResizeEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow;}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    Player* player;

protected:
    void moveEvent(QMoveEvent* event)Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent* event)Q_DECL_OVERRIDE;
};
#endif // MAINWINDOW_H
