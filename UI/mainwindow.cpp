#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "player.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    player = new Player(this);
    this->setWindowTitle("唬人播放器");
    this->setCentralWidget(player);
    setAcceptDrops(true);
    this->setStyleSheet("QMainWindow {background-color:black}");
    this->setMouseTracking(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::moveEvent(QMoveEvent *event) {
    this->player->Move();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    this->player->Move();
}

