#include "widget.h"
#include "./ui_widget.h"

#include <QPushButton>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    player=AVPlayer::getInstance();
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_clicked()
{
    QString url("../media-demo-Cen/Resources/aaa.mkv");
    player->Start(url,(void*)ui->label->winId());
}


void Widget::on_exit_clicked()
{
    player->SetPlayMode(-1);
}


void Widget::on_resume_clicked()
{
    player->SetPos(40.0,1);
}


void Widget::on_show_clicked()
{
    player->Close();
}

