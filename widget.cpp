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



void Widget::on_start_clicked()
{
    QString url("D:/Thunder/电影/阳光电影www.ygdy8.com.月光骑士第一季第03集中英双字.mkv");
    player->Start(url,(void*)ui->label->winId());
}

void Widget::on_stop_clicked()
{
    player->Stop();
}


void Widget::on_resume_clicked()
{
    player->Resume();
}




void Widget::on_close_clicked()
{
    player->Close();
}




void Widget::on_setspeed_clicked()
{
    player->SetPlaySpeed(2.2);
}


void Widget::on_setpos_clicked()
{
    player->SetPos(239);
}


void Widget::on_setmode_clicked()
{
    player->SetPlayMode(-1);
}

